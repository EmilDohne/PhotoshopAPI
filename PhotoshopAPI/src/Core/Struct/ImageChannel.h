#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Profiling/Perf/Instrumentor.h"
#include "Profiling/Memory/CompressionTracker.h"
#include "PhotoshopFile/FileHeader.h"
#include "CoordinateUtil.h"
#include "ThreadPool.h"

#include "blosc2.h"

#include <vector>
#include <thread>
#include <memory>
#include <random>
#include <execution>
#include <cassert>

// If we compile with C++<20 we replace the stdlib implementation with the compatibility
// library
#if (__cplusplus < 202002L)
#include "tcb_span.hpp"
#else
#include <span>
#endif


#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include <cmath>

PSAPI_NAMESPACE_BEGIN


/// A generic Image Channel that is used by both the PhotoshopFile and LayeredFile, being moved between these two
/// It is entirely valid to have each channel have a different compression method, width and height. We only
/// store the image data in here but do not deal with reading or writing it. Ownership of the image data belongs
/// to this struct and gets freed on destruction or extraction
struct ImageChannel
{
	/// The size of each sub-chunk in the super-chunk. For more information about what a chunk and super-chunk is
	/// please refer to the c-blosc2 documentation. Defaults to 8MB
	static const uint64_t m_ChunkSize = 1024 * 1024 * 8;
	/// This does not indicate the compression method of the channel in memory 
	/// but rather the compression method it writes the PhotoshopFile with
	Enum::Compression m_Compression = Enum::Compression::Raw;
	/// Information about what channel this actually is
	Enum::ChannelIDInfo m_ChannelID = { Enum::ChannelID::Red, 1 };
	/// The size of the original (uncompressed) data in bytes
	uint64_t m_OrigByteSize = 0u;	


	/// Get the width of the uncompressed ImageChannel
	int32_t getWidth() const { return m_Width; };
	/// Get the height of the uncompressed ImageChannel
	int32_t getHeight() const { return m_Height; };
	/// Get the x-coordinate of the uncompressed ImageChannel
	float getCenterX() const { return m_XCoord; };
	/// Get the y-coordinate of the uncompressed ImageChannel
	float getCenterY() const { return m_YCoord; };
	/// Get the total number of chunks held in the ImageChannel
	uint64_t getNumChunks() const { return m_NumChunks; };


	/// Extract the data from the image channel and invalidate it (can only be called once). 
	/// If the image data does not exist yet we simply return an empty vector<T>. If the data
	/// was already freed we throw
	/// 
	/// \param numThreads The number of threads to use for decompression. By default this is 0 which will set it to hardware_concurrency.
	///					  If you are calling this in a non-threaded environment this is likely the option you should choose
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	std::vector<T> extractData(size_t numThreads = 0) {
		PROFILE_FUNCTION();
		auto buffer = getData<T>(numThreads);
		if (buffer.size() > 0)
		{
			blosc2_schunk_free(m_Data);
			m_wasFreed = true;
			return buffer;
		}
		return std::vector<T>();
	}

	/// Extract the data from the image channel and invalidate it (can only be called once). 
	/// If the image data does not exist yet we simply return an empty vector<T>. If the data
	/// was already freed we throw
	/// 
	/// \param buffer A preallocated buffer whose size matches that of m_OrigByteSize / sizeof(T). If this is not given we throw an error
	/// \param numThreads The number of threads to use for decompression. By default this is 0 which will set it to hardware_concurrency.
	///					  If you are calling this in a non-threaded environment this is likely the option you should choose
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	void extractData(std::span<T> buffer, size_t numThreads = 0) {
		PROFILE_FUNCTION();

		if (!m_Data)
		{
			PSAPI_LOG_WARNING("ImageChannel", "extractData() called without the channel having been initialized yet, returning without having filled the given buffer");
			return;
		}

		getData<T>(buffer, numThreads);
		blosc2_schunk_free(m_Data);
		m_wasFreed = true;
	}


	/// Copy the image data out of the ImageChannel using a preallocated buffer, does not free the data afterwards. If the data was already freed we throw
	/// 
	/// \param buffer A preallocated buffer whose size matches that of m_OrigByteSize / sizeof(T). If this is not given we throw an error
	/// \param numThreads The number of threads to use for decompression. By default this is 0 which will set it to hardware_concurrency.
	///					  If you are calling this in a non-threaded environment this is likely the option you should choose
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	void getData(std::span<T> buffer, size_t numThreads = 0)
	{
		PROFILE_FUNCTION();

		if (!m_Data)
		{
			PSAPI_LOG_WARNING("ImageChannel", "Channel data does not exist yet, was it initialized?");
			return;
		}
		if (m_wasFreed)
		{
			PSAPI_LOG_ERROR("ImageChannel", "Data was already freed, cannot extract it anymore");
		}
		if (buffer.size() != m_OrigByteSize / sizeof(T))
		{
			PSAPI_LOG_ERROR("ImageChannel", "getData() buffer must be exactly the size of m_OrigByteSize / sizeof(T)");
		}

		// Set thread number to hardware concurrency if we dont detect threading
		if (numThreads == 0)
		{
			numThreads = std::thread::hardware_concurrency();
		}

		Internal::ThreadPool pool(numThreads);

		uint64_t remainingSize = m_OrigByteSize;
		std::vector<blosc2_context*> contexts;
		std::vector<std::future<void>> futures; // To store future objects for each task

		blosc2_dparams params = BLOSC2_DPARAMS_DEFAULTS;

		for (uint64_t nchunk = 0; nchunk < m_NumChunks; ++nchunk)
		{
			// Create a unique context
			contexts.emplace_back(blosc2_create_dctx(*m_Data->storage->dparams));

			void* ptr = reinterpret_cast<uint8_t*>(buffer.data()) + nchunk * m_ChunkSize;
			if (remainingSize > m_ChunkSize)
			{
				futures.emplace_back(pool.enqueue([=, this]() {
					blosc2_decompress_ctx(
						contexts.back(),
						m_Data->data[nchunk], 
						std::numeric_limits<int32_t>::max(), 
						ptr, 
						m_ChunkSize);
					}));
				remainingSize -= m_ChunkSize;
			}
			else
			{
				assert(remainingSize < std::numeric_limits<int32_t>::max());
				futures.emplace_back(pool.enqueue([=, this]() {
					blosc2_decompress_ctx(
						contexts.back(),
						m_Data->data[nchunk], 
						std::numeric_limits<int32_t>::max(), 
						ptr, 
						static_cast<int32_t>(remainingSize));
					}));
				remainingSize = 0;
			}
		}

		// Wait for all tasks to complete
		for (auto& future : futures) {
			future.wait();
		}

		// Free the decompression contexts
		for (auto* ctx : contexts)
		{
			blosc2_free_ctx(ctx);
		}
	}


	/// Copy the image data out of the ImageChannel, does not free the data afterwards. Returns an empty vector if the
	/// data does not exist yet. If the data was already freed we throw
	/// 
	/// \param numThreads The number of threads to use for decompression. By default this is 0 which will set it to hardware_concurrency.
	///					  If you are calling this in a non-threaded environment this is likely the option you should choose
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	std::vector<T> getData(size_t numThreads = 0)
	{
		std::vector<T> buffer(m_OrigByteSize / sizeof(T));
		getData(std::span<T>(buffer), numThreads);
		return buffer;
	}


	/// Extract n amount of randomly selected chunks from the ImageChannel super chunk. This does not invalidate any data
	/// and is useful to e.g. compress these chunks using photoshops compression methods to estimate the final size on disk
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	std::vector<std::vector<T>> getRandomChunks(const FileHeader header, uint16_t numChunks) const
	{
		std::random_device rd;
		std::mt19937 randomEngine(rd());
		// We dont really want to deal with partial chunks so we simply ignore the last chunk.
		// Since the range is inclusive we subtract 2
		std::uniform_int_distribution<> dist(0, m_NumChunks - 2);

		std::vector<std::vector<T>> outChunks;
		outChunks.reserve(numChunks);

		for (int i = 0; i < numChunks; ++i)
		{
			std::vector<T> decompressedChunk(m_ChunkSize, 0u);
			void* ptr = reinterpret_cast<void*>(decompressedChunk.data());
			// Decompress a random chunk into decompressedChunk
			blosc2_schunk_decompress_chunk(m_Data, dist(randomEngine), ptr, m_ChunkSize);
			outChunks.push_back(decompressedChunk);
		}
		// Note that we do not call blosc2_schunk_free() here to keep the data valid
		return outChunks;
	}


	/// Take a reference to a decompressed image vector and initialize the blosc2 superchunk 
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	ImageChannel(Enum::Compression compression, const std::vector<T>& imageData, const Enum::ChannelIDInfo channelID, const int32_t width, const int32_t height, const float xcoord, const float ycoord)
	{
		if (width > 300000u)
			PSAPI_LOG_ERROR("ImageChannel", "Invalid width parsed to image channel. Photoshop channels can be 300,000 pixels wide, got %" PRIu32 " instead",
				width);
		if (height > 300000u)
			PSAPI_LOG_ERROR("ImageChannel", "Invalid height parsed to image channel. Photoshop channels can be 300,000 pixels high, got %" PRIu32 " instead",
				height);
		m_Compression = compression;
		m_Width = width;
		m_Height = height;
		m_XCoord = xcoord;
		m_YCoord = ycoord;
		m_ChannelID = channelID;
		if (imageData.size() != static_cast<uint64_t>(width) * height) [[unlikely]]
		{
			PSAPI_LOG_ERROR("ImageChannel", "provided imageData does not match the expected size of %" PRIu64 " but is instead %i", static_cast<uint64_t>(width) * height, imageData.size());
		}
		initializeBlosc2Schunk<T>(std::span<const T>(imageData.begin(), imageData.end()), width, height);
	}


	/// Take a reference to a decompressed image span and initialize the blosc2 superchunk 
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	ImageChannel(Enum::Compression compression, const std::span<T> imageData, const Enum::ChannelIDInfo channelID, const int32_t width, const int32_t height, const float xcoord, const float ycoord)
	{
		if (width > 300000u)
			PSAPI_LOG_ERROR("ImageChannel", "Invalid width parsed to image channel. Photoshop channels can be 300,000 pixels wide, got %" PRIu32 " instead",
				width);
		if (height > 300000u)
			PSAPI_LOG_ERROR("ImageChannel", "Invalid height parsed to image channel. Photoshop channels can be 300,000 pixels high, got %" PRIu32 " instead",
				height);
		m_Compression = compression;
		m_Width = width;
		m_Height = height;
		m_XCoord = xcoord;
		m_YCoord = ycoord;
		m_ChannelID = channelID;
		if (imageData.size() != static_cast<uint64_t>(width) * height) [[unlikely]]
			PSAPI_LOG_ERROR("ImageChannel", "provided imageData does not match the expected size of %" PRIu64 " but is instead %i", static_cast<uint64_t>(width) * height, imageData.size());
		initializeBlosc2Schunk(std::span<const T>(imageData.begin(), imageData.end()), width, height);
	}


	/// Take a reference to a decompressed image span and initialize the blosc2 superchunk 
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	ImageChannel(Enum::Compression compression, const std::span<const T> imageData, const Enum::ChannelIDInfo channelID, const int32_t width, const int32_t height, const float xcoord, const float ycoord)
	{
		if (width > 300000u)
			PSAPI_LOG_ERROR("ImageChannel", "Invalid width parsed to image channel. Photoshop channels can be 300,000 pixels wide, got %" PRIu32 " instead",
				width);
		if (height > 300000u)
			PSAPI_LOG_ERROR("ImageChannel", "Invalid height parsed to image channel. Photoshop channels can be 300,000 pixels high, got %" PRIu32 " instead",
				height);
		m_Compression = compression;
		m_Width = width;
		m_Height = height;
		m_XCoord = xcoord;
		m_YCoord = ycoord;
		m_ChannelID = channelID;
		if (imageData.size() != static_cast<uint64_t>(width) * height) [[unlikely]]
			PSAPI_LOG_ERROR("ImageChannel", "provided imageData does not match the expected size of %" PRIu64 " but is instead %i", static_cast<uint64_t>(width) * height, imageData.size());
			initializeBlosc2Schunk(imageData, width, height);
	}


	// On destruction free the blosc2 schunk if it wasnt freed yet
	~ImageChannel() 
	{
		if (!m_wasFreed)
			blosc2_schunk_free(m_Data);
		m_wasFreed = true;
	}
	ImageChannel() = default;


private:

	blosc2_schunk* m_Data = nullptr;
	/// Total number of chunks in the super-chunk
	uint64_t m_NumChunks = 0u;
	/// Whether or not the SuperChunk was freed, if this is true the image data is no longer valid
	bool m_wasFreed = false;
	// Photoshop stores their positions as a bounding rect but we instead store extents and center coordinates
	int32_t m_Width = 0u;
	int32_t m_Height = 0u;
	float m_XCoord = 0.0f;
	float m_YCoord = 0.0f;

private:

	// Initialize a blosc2 superchunk from a given data span, maybe we could augment this to give control over compression params?
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T> 
	void initializeBlosc2Schunk(const std::span<const T> imageData, const int32_t width, const int32_t height)
	{
		PROFILE_FUNCTION();
		m_OrigByteSize = static_cast<uint64_t>(width) * height * sizeof(T);

		blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
		blosc2_dparams dparams = BLOSC2_DPARAMS_DEFAULTS;
		// Calculate the number of chunks from the input
		uint64_t numChunks = static_cast<uint64_t>(ceil((static_cast<double>(width) * height * sizeof(T)) / m_ChunkSize));
		// This could either be a no-op or a chunk that is smaller than m_ChunkSize
		if (numChunks == 0)
		{
			// Check for chunks that are smaller than m_ChunkSize
			if (static_cast<uint64_t>(width) * height * sizeof(T) > 0)
			{
				numChunks = 1;
			}
		}
		m_NumChunks = numChunks;

		// Set parameters to help with compression and decompression
		cparams.typesize = sizeof(T);
		cparams.compcode = BLOSC_LZ4;
		cparams.clevel = 5;
		// Running this on a single thread speeds up execution since we already are running across multiple threads
		// on decoding of the layers
		cparams.nthreads = 1;
		dparams.nthreads = 1;
		blosc2_storage storage = { .cparams = &cparams, .dparams = &dparams };

		// Initialize our schunk
		m_Data = blosc2_schunk_new(&storage);

		uint64_t remainingSize = static_cast<uint64_t>(width) * height * sizeof(T);
		for (int nchunk = 0; nchunk < numChunks; ++nchunk)
		{
			const void* ptr = reinterpret_cast<const uint8_t*>(imageData.data()) + nchunk * m_ChunkSize;
			int64_t nchunks;
			if (remainingSize > m_ChunkSize)
			{
				// C-blosc2 returns the total number of chunks here. We const cast as the function does not 
				// modify the data yet takes a void* rather than a const void*
				nchunks = blosc2_schunk_append_buffer(m_Data, const_cast<void*>(ptr), m_ChunkSize);
				remainingSize -= m_ChunkSize;
			}
			else
			{
				assert(remainingSize < std::numeric_limits<int32_t>::max());
				// C-blosc2 returns the total number of chunks here. We const cast as the function does not 
				// modify the data yet takes a void* rather than a const void*
				nchunks = blosc2_schunk_append_buffer(m_Data, const_cast<void*>(ptr), static_cast<int32_t>(remainingSize));
				remainingSize = 0;
			}
			if (nchunks != nchunk + 1) [[unlikely]]
			{
				PSAPI_LOG_ERROR("ImageChannel", "Unexpected number of chunks");
			}
		}
		// Log the total compressed / uncompressed size to later determine our stats
		REGISTER_COMPRESSION_TRACK(static_cast<uint64_t>(m_Data->cbytes), static_cast<uint64_t>(m_Data->nbytes));
	}
};


PSAPI_NAMESPACE_END