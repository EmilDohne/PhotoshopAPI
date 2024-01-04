#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Profiling/Perf/Instrumentor.h"
#include "Profiling/Memory/CompressionTracker.h"

#include "blosc2.h"

#include <vector>
#include <thread>
#include <memory>
#include <random>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include <cmath>

PSAPI_NAMESPACE_BEGIN


struct BaseImageChannel
{
	Enum::Compression m_Compression = Enum::Compression::Raw;
	Enum::ChannelIDInfo m_ChannelID = { Enum::ChannelID::Red, 1 };
	uint64_t m_OrigByteSize = 0u;	// The size of the original vector in bytes


	BaseImageChannel() = default;
	BaseImageChannel(Enum::Compression compression, const Enum::ChannelIDInfo channelID, const int32_t width, const int32_t height, const int32_t xcoord, const int32_t ycoord)
	{
		if (width > 300000u)
		{
			PSAPI_LOG_ERROR("ImageChannel", "Invalid width parsed to image channel. Photoshop channels can be 300,000 pixels wide, got %" PRIu32 " instead",
				width)
		}
		if (height > 300000u)
		{
			PSAPI_LOG_ERROR("ImageChannel", "Invalid height parsed to image channel. Photoshop channels can be 300,000 pixels high, got %" PRIu32 " instead",
				height)
		}

		m_Compression = compression;
		m_Width = width;
		m_Height = height;
		m_XCoord = xcoord;
		m_YCoord = ycoord;
		m_ChannelID = channelID;
	}

	virtual ~BaseImageChannel() = default;

	int32_t getWidth() const { return m_Width; };
	int32_t getHeight() const { return m_Height; };
	int32_t getCenterX() const { return m_XCoord; };
	int32_t getCenterY() const { return m_YCoord; };

protected:
	// Photoshop stores their positions as a bounding rect but we instead store extents and center coordinates
	int32_t m_Width = 0u;
	int32_t m_Height = 0u;
	int32_t m_XCoord = 0u;
	int32_t m_YCoord = 0u;
};


// A generic Image Channel that could either be part of the Channel Image Data section or Image Data section
// It is entirely valid to have each channel have a different compression method, width and height. We only
// store the image data in here but do not deal with reading or writing it. Ownership of the image data belongs
// to this struct
template <typename T>
struct ImageChannel : public BaseImageChannel
{
	static const uint32_t m_ChunkSize = 1024 * 1024;	// Size of each individual chunk in the schunk

	ImageChannel() = default;
	// Take a reference to a decompressed image vector stream and set the according member variables
	ImageChannel(Enum::Compression compression, std::vector<T> imageData, const Enum::ChannelIDInfo channelID, const int32_t width, const int32_t height, const int32_t xcoord, const int32_t ycoord) :
		BaseImageChannel(compression, channelID, width, height, xcoord, ycoord)
	{

		PROFILE_FUNCTION();
		m_OrigSize = static_cast<uint64_t>(width) * height;
		m_OrigByteSize = static_cast<uint64_t>(width) * height * sizeof(T);

		blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
		blosc2_dparams dparams = BLOSC2_DPARAMS_DEFAULTS;
		// Calculate the number of chunks from the input
		int numChunks = ceil((static_cast<uint64_t>(width) * height * sizeof(T)) / m_ChunkSize);
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
		// TODO set this to hardware concurrency?
		cparams.nthreads = 4;
		dparams.nthreads = 4;
		blosc2_storage storage = {.contiguous=true, .cparams = &cparams, .dparams = &dparams };

		// Initialize our schunk
		m_Data = blosc2_schunk_new(&storage);

		uint64_t remainingSize = static_cast<uint64_t>(width) * height * sizeof(T);
		for (int nchunk = 0; nchunk < numChunks; ++nchunk)
		{
			// Cast to uint8_t* to iterate by bytes, not by T
			void* ptr = reinterpret_cast<uint8_t*>(imageData.data()) + nchunk * m_ChunkSize;
			int64_t nchunks;
			if (remainingSize > m_ChunkSize)
			{
				nchunks = blosc2_schunk_append_buffer(m_Data, ptr, m_ChunkSize);
				remainingSize -= m_ChunkSize;
			}
			else
			{
				nchunks = blosc2_schunk_append_buffer(m_Data, ptr, remainingSize);
				remainingSize = 0;
			}
			if (nchunks != nchunk + 1)
			{
				PSAPI_LOG_ERROR("ImageChannel", "Unexpected number of chunks")
			}
		}

		// Log the total compressed / uncompressed size to later determine our stats
		REGISTER_COMPRESSION_TRACK(static_cast<uint64_t>(m_Data->cbytes), static_cast<uint64_t>(m_Data->nbytes));
	};


	// Extract the data from the image channel and invalidate it (can only be called once). If the image data does not exist yet we simply return an empty vector<T>
	std::vector<T> getData() {
		PROFILE_FUNCTION();
		if (!m_Data)
		{
			return std::vector<T>();
		}

		std::vector<T> tmpData(m_OrigSize, 0);

		uint64_t remainingSize = m_OrigSize;

		for (uint32_t nchunk = 0; nchunk < m_NumChunks; ++nchunk)
		{
			void* ptr = reinterpret_cast<void*>(tmpData.data() + nchunk * m_ChunkSize);
			if (remainingSize < m_ChunkSize)
			{
				blosc2_schunk_decompress_chunk(m_Data, nchunk, ptr, m_ChunkSize);
				remainingSize -= m_ChunkSize;
			}
			else
			{
				blosc2_schunk_decompress_chunk(m_Data, nchunk, ptr, remainingSize);
				remainingSize = 0;
			}
		}

		blosc2_schunk_free(m_Data);

		return tmpData;
	}

	// Extract n amount of randomly selected chunks from the ImageChannel super chunk. This does not invalidate any data
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

	uint32_t getNumChunks() const  { return m_NumChunks };

private:
	blosc2_schunk* m_Data = nullptr;
	uint32_t m_NumChunks = 0u;

};


PSAPI_NAMESPACE_END