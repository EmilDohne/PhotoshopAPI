#pragma once

#include "Macros.h"
#include "Read.h"
#include "Logger.h"
#include "Endian/EndianByteSwap.h"
#include "Endian/EndianByteSwapArr.h"
#include "Struct/File.h"
#include "Struct/ByteStream.h"
#include "PhotoshopFile/FileHeader.h"
#include "Profiling/Perf/Instrumentor.h"

#include <vector>

#include <inttypes.h>


PSAPI_NAMESPACE_BEGIN

// This is the packbits algorithm described here: https://en.wikipedia.org/wiki/PackBits
// We iterate byte by byte and decompress
template<typename T>
std::vector<uint8_t> DecompressPackBits(const std::vector<uint8_t>& compressedData, const uint32_t width, const uint32_t height)
{
    PROFILE_FUNCTION();
    std::vector<uint8_t> decompressedData;
    decompressedData.reserve((sizeof(T) * static_cast<uint64_t>(width) * static_cast<uint64_t>(height)));

    uint64_t i = 0;
    while (i < compressedData.size()) {
        uint8_t value = compressedData[i];

        if (value == 128) 
        {
            // Do nothing, nop. Equivalent to 0 in int8_t
        }
        else if (value > 128) 
        {
            // Repeat the next byte after this n times
            value = 256 - value;
            for (int j = 0; j <= value; ++j)
            {
                decompressedData.push_back(compressedData.at(i + 1));
            }
            ++i;
        }
        else 
        {
            // Header byte indicates the next n bytes are to be read as values
            for (int j = 0; j <= value; ++j)
            {
                decompressedData.push_back(compressedData.at(i + j + 1));
            }
            i += static_cast<uint64_t>(value) + 1;
        }
        ++i;
    }

    return decompressedData;
}


// Reads and decompresses a single channel using the packbits algorithm
template<typename T>
std::vector<T> DecompressRLE(ByteStream& stream, const FileHeader& header, const uint32_t width, const uint32_t height, const uint64_t compressedSize)
{
    PROFILE_FUNCTION();
	// Photoshop first stores the byte counts of all the scanlines, this is 2 or 4 bytes depending on 
	// if the document is PSD or PSB
	uint64_t scanlineTotalSize = 0u;
	for (int i = 0; i < height; ++i)
	{
		scanlineTotalSize += ExtractWidestValue<uint16_t, uint32_t>(ReadBinaryDataVariadic<uint16_t, uint32_t>(stream, header.m_Version));
	}

    // Find out the size of the data without the scanline sizes. For example, if the document is 64x64 pixels in 8 bit mode we have 128 bytes of memory to store the scanline size
    uint64_t dataSize = compressedSize - static_cast<uint64_t>(SwapPsdPsb<uint16_t, uint32_t>(header.m_Version)) * height;

    if (scanlineTotalSize != dataSize)
    {
        PSAPI_LOG_ERROR("DecompressRLE", "Size of compressed data is not what was expected. Expected: %" PRIu64 " but got %" PRIu64 " instead",
            dataSize,
            scanlineTotalSize)
    }

	// Read the data without converting from BE to native as we need to decompress first
	std::vector<uint8_t> compressedData(scanlineTotalSize);
    stream.read(reinterpret_cast<char*>(compressedData.data()), scanlineTotalSize);

	// Decompress using the PackBits algorithm
    std::vector<uint8_t> decompressedData = DecompressPackBits<T>(compressedData, width, height);

    // Convert decompressed data to native endianness
    std::vector<T> bitShiftedData = endianDecodeBEBinaryArray<T>(decompressedData);


    if (bitShiftedData.size() != static_cast<uint64_t>(width) * static_cast<uint64_t>(height))
    {
        PSAPI_LOG_ERROR("DecompressRLE", "Size of decompressed data is not what was expected. Expected: %" PRIu64 " but got %" PRIu64 " instead", 
            static_cast<uint64_t>(width) * static_cast<uint64_t>(height),
            bitShiftedData.size())
    }
	
	return bitShiftedData;
}


PSAPI_NAMESPACE_END