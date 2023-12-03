#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Profiling/Perf/Instrumentor.h"

#include "blosc2.h"

#include <vector>
#include <thread>
#include <memory>

#include <cmath>

PSAPI_NAMESPACE_BEGIN


struct BaseImageChannel
{
	Enum::Compression m_Compression = Enum::Compression::Raw;
	Enum::ChannelID m_ChannelID = Enum::ChannelID::Red;

	BaseImageChannel(Enum::Compression compression, const Enum::ChannelID channelID, const uint32_t width, const uint32_t height)
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
		m_ChannelID = channelID;
	}

	virtual ~BaseImageChannel() = default;

protected:
	uint32_t m_Width = 0u;
	uint32_t m_Height = 0u;
};


// A generic Image Channel that could either be part of the Channel Image Data section or Image Data section
// It is entirely valid to have each channel have a different compression method, width and height. We only
// store the image data in here but do not deal with reading or writing it. Ownership of the image data belongs
// to this struct
template <typename T>
struct ImageChannel : public BaseImageChannel
{
	// Take a reference to a decompressed image vector stream and set the according member variables
	ImageChannel(Enum::Compression compression, std::vector<T> imageData, const Enum::ChannelID channelID, const uint32_t width, const uint32_t height) : BaseImageChannel(compression, channelID, width, height)
	{
		PROFILE_FUNCTION();
		m_OrigSize = static_cast<uint64_t>(width) * height;

		blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
		blosc2_dparams dparams = BLOSC2_DPARAMS_DEFAULTS;
		// Calculate the number of chunks from the input
		int numChunks = ceil((static_cast<uint64_t>(width) * height * sizeof(T)) / m_ChunkSize);
		
		// Set parameters to help with compression and decompression
		cparams.typesize = sizeof(T);
		cparams.clevel = 9;
		cparams.nthreads = 1;
		dparams.nthreads = 1;
		blosc2_storage storage = { .cparams = &cparams, .dparams = &dparams };

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
	};


	std::vector<T> getData() {
		PROFILE_FUNCTION();
		std::vector<T> tmpData(m_OrigSize, 0);
		bool needsFree = false;

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

private:
	blosc2_schunk* m_Data;
	uint32_t m_ChunkSize = 1000 * 10000;	// Size of each individual chunk in the schunk
	uint32_t m_NumChunks;
	uint64_t m_OrigSize;	// Original vector size, not in terms of bytes but in terms of elements. E.g. in a 64x64 pixel 16 bit file this would be 4,096, not 8192

};

PSAPI_NAMESPACE_END