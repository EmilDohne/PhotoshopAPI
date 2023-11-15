#pragma once

#include "../../Macros.h"
#include "../Read.h"
#include "../EndianByteSwap.h"
#include "../Struct/File.h"
#include "../../PhotoshopFile/FileHeader.h"
#include "../Logger.h"

#include <vector>

#include <inttypes.h>


PSAPI_NAMESPACE_BEGIN

// This is the packbits algorithm described here: https://en.wikipedia.org/wiki/PackBits
// We iterate byte by byte and decompress
template<typename T>
std::vector<uint8_t> DecompressPackBits(const std::vector<uint8_t>& compressedData, const uint32_t width, const uint32_t height)
{
    std::vector<uint8_t> decompressedData;
    decompressedData.reserve((sizeof(T) * static_cast<uint64_t>(width) * static_cast<uint64_t>(height)));

    uint64_t i = 0;
    while (i < compressedData.size()) {
        uint8_t value = compressedData[i];

        if (value == 128) {
            // Do nothing, nop
        }
        else if (value > 128) {
            // Repeated byte
            value = 256 - value;
            for (int j = 0; j <= value; ++j)
            {
                decompressedData.push_back(compressedData.at(i + 1));
            }
            ++i;
        }
        else {
            // Literal bytes
            for (int j = 0; j <= value; ++j)
            {
                decompressedData.push_back(compressedData.at(i + j + 1));
            }
            i += value + 1;
        }
        ++i;
    }

    return decompressedData;
}


// Reads and decompresses a single channel using the packbits algorithm
template<typename T>
std::vector<T> DecompressRLE(File& document, const FileHeader& header, const uint32_t width, const uint32_t height, const uint64_t compressedSize)
{
	// Photoshop first stores the byte counts of all the scanlines, this is 2 or 4 bytes depending on 
	// if the document is PSD or PSB
	uint64_t scanlineTotalSize = 0u;
	for (int i = 0; i < height; ++i)
	{
		scanlineTotalSize += ExtractWidestValue<uint16_t, uint32_t>(ReadBinaryDataVariadic<uint16_t, uint32_t>(document));
	}

    if (scanlineTotalSize != (compressedSize + static_cast<uint64_t>(height) * SwapPsdPsb<uint16_t, uint32_t>(header.m_Version))
    {
        PSAPI_LOG_ERROR("DecompressRLE", "Size of compressed data is not what was expected. Expected: " % PRIu64 " but got " % PRIu64 " instead",
            compressedSize + static_cast<uint64_t>(height) * SwapPsdPsb<uint16_t, uint32_t>(header.m_Version),
            scanlineTotalSize)
    }

	// Read the data without converting from BE to native as we need to decompress first
	std::vector<uint8_t> compressedData(scanlineTotalSize);
	document.read(reinterpret_cast<char*>(compressedData.data()), scanlineTotalSize);

	// Decompress using the PackBits algorithm
    std::vector<uint8_t> decompressedData = DecompressPackBits<T>(compressedData);

    // Convert decompressed data to native endianness
    std::vector<T> bitShiftedData = endianDecodeBEBinaryArray(decompressedData);


    if (bitShiftedData.size() != static_cast<uint64_t>(width) * static_cast<uint64_t>(height))
    {
        PSAPI_LOG_ERROR("DecompressRLE", "Size of decompressed data is not what was expected. Expected: " %PRIu64 " but got " %PRIu64 " instead", 
            static_cast<uint64_t>(width) * static_cast<uint64_t>(height),
            bitShiftedData.size())
    }
	
	return bitShiftedData;
};


PSAPI_NAMESPACE_END