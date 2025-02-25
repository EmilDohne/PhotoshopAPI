#pragma once

#include "Macros.h"
#include "Util/Logger.h"
#include "Core/FileIO/Util.h"
#include "Core/Endian/EndianByteSwap.h"
#include "Core/Endian/EndianByteSwapArr.h"
#include "Core/Struct/File.h"
#include "Core/Struct/ByteStream.h"
#include "PhotoshopFile/FileHeader.h"
#include "Util/Profiling/Perf/Instrumentor.h"
#include "Util/FileUtil.h"

#include <vector>
#include <limits>

#include <cstring>
#include <inttypes.h>


#ifdef __AVX2__
#include "Decompress_RLE_AVX2.h"
#endif

// If we compile with C++<20 we replace the stdlib implementation with the compatibility
// library
#if (__cplusplus < 202002L)
#include "tcb_span.hpp"
#else
#include <span>
#endif

PSAPI_NAMESPACE_BEGIN


namespace RLE_Impl
{
    // This is the packbits algorithm described here: https://en.wikipedia.org/wiki/PackBits we iterate byte by byte and decompress
    // a singular scanline at a time
    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    template<typename T>
    std::vector<uint8_t> DecompressPackBits(std::span<const uint8_t> compressedData, const uint32_t width, const uint32_t height)
    {
        PSAPI_PROFILE_FUNCTION();
        std::vector<uint8_t> decompressedData(sizeof(T) * static_cast<uint64_t>(width) * static_cast<uint64_t>(height), 0u);

        uint64_t i = 0;
        uint64_t idx = 0;   // Index into decompressedData
        const auto dataSize = compressedData.size();

        while (i < dataSize) {
            const uint8_t value = compressedData[i];

            if (value == 128) [[unlikely]]
            {
                // Do nothing, nop. Equivalent to 0 in int8_t
            }
            else if (value > 128)
            {
                // Repeat the next byte after this 257-n times
                const uint8_t repeat_val = compressedData[i + 1];
                for (int j = 0; j <= 256 - value; ++j)
                {
                    decompressedData[idx] = repeat_val;
                    ++idx;
                }
                ++i;
            }
            else
            {
                // Header byte indicates the next n bytes are to be read as values
                for (int j = 0; j <= value; ++j)
                {
                    decompressedData[idx] = compressedData[i + j + 1];
                    ++idx;
                }
                i += value + 1;
            }
            ++i;
        }

        return decompressedData;
    }


    // This is the packbits algorithm described here: https://en.wikipedia.org/wiki/PackBits we iterate byte by byte and decompress
    // a singular scanline at a time
    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    template<typename T>
    void DecompressPackBits(std::span<const uint8_t> compressedData, std::span<uint8_t> decompressedData)
    {
        PSAPI_PROFILE_FUNCTION();

        uint64_t i = 0;
        uint64_t idx = 0;   // Index into decompressedData
        const auto dataSize = compressedData.size();

        while (i < dataSize) {
            const uint8_t value = compressedData[i];

            if (value == 128) [[unlikely]]
                {
                    // Do nothing, nop. Equivalent to 0 in int8_t
                }
            else if (value > 128)
            {
                // Repeat the next byte after this 257-n times
                const uint8_t repeat_val = compressedData[i + 1];
                for (int j = 0; j <= 256 - value; ++j)
                {
                    decompressedData[idx] = repeat_val;
                    ++idx;
                }
                ++i;
            }
            else
            {
                // Header byte indicates the next n bytes are to be read as values
                for (int j = 0; j <= value; ++j)
                {
                    decompressedData[idx] = compressedData[i + j + 1];
                    ++idx;
                }
                i += value + 1;
            }
            ++i;
        }
    }

}


// Reads and decompresses a single channel using the packbits algorithm into the provided buffer. Buffer must be at least
// large enough to hold width * height * sizeof(T)
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<typename T>
void DecompressRLE(ByteStream& stream, std::span<T> buffer, uint64_t offset, const FileHeader& header, const uint32_t width, const uint32_t height, const uint64_t compressedSize)
{
    PSAPI_PROFILE_FUNCTION();

    if (buffer.size() < static_cast<uint64_t>(width) * static_cast<uint64_t>(height))
    {
        PSAPI_LOG_ERROR("DecompressRLE", "Provided buffer is not large enough. Expected at least: %" PRIu64 " but got %" PRIu64 " instead",
            static_cast<uint64_t>(width) * static_cast<uint64_t>(height),
            buffer.size());
    }

    // Photoshop first stores the byte counts of all the scanlines, this is 2 or 4 bytes depending on 
    // if the document is PSD or PSB
    uint64_t scanlineTotalSize = 0u;
    std::vector<uint32_t> scanlineSizes;
    if (header.m_Version == Enum::Version::Psd)
    {
        std::vector<uint16_t> buff(height);
        stream.read(Util::toWritableBytes(buff), offset);
        endianDecodeBEArray<uint16_t>(buff);
        for (auto item : buff)
        {
            scanlineTotalSize += item;
            scanlineSizes.push_back(item);
        }
    }
    else
    {
        std::vector<uint32_t> buff(height);
        stream.read(Util::toWritableBytes(buff), offset);
        endianDecodeBEArray<uint32_t>(buff);
        for (auto item : buff)
        {
            scanlineTotalSize += item;
            scanlineSizes.push_back(item);
        }
    }

    // Find out the size of the data without the scanline sizes. For example, if the document is 64x64 pixels in 8 bit mode we have 128 bytes of memory to store the scanline size
    uint64_t dataSize = compressedSize - static_cast<uint64_t>(SwapPsdPsb<uint16_t, uint32_t>(header.m_Version)) * height;

    if (scanlineTotalSize != dataSize)
    {
        PSAPI_LOG_ERROR("DecompressRLE", "Size of compressed data is not what was expected. Expected: %" PRIu64 " but got %" PRIu64 " instead",
            dataSize,
            scanlineTotalSize);
    }

    // Read the data without converting from BE to native as we need to decompress first
    std::span<uint8_t> compressedData = stream.read(offset + SwapPsdPsb<uint16_t, uint32_t>(header.m_Version) * height, scanlineTotalSize);

    // Generate spans for every individual scanline to decompress them individually
    std::vector<std::span<const uint8_t>> compressedDataSpans(height);
    std::vector<std::span<uint8_t>> decompressedDataSpans(height);
    std::vector<uint64_t> verticalIter;
    {
        uint64_t compressedStartIdx = 0;
        for (uint64_t i = 0; i < static_cast<uint64_t>(height); ++i)
        {
            uint64_t compressedEndIdx = compressedStartIdx + scanlineSizes[i];
            compressedDataSpans[i] = std::span<const uint8_t>(compressedData.begin() + compressedStartIdx, compressedData.begin() + compressedEndIdx);
            compressedStartIdx = compressedEndIdx;

            uint64_t decompressedStartIdx = i * width * sizeof(T);
            decompressedDataSpans[i] = std::span<uint8_t>(reinterpret_cast<uint8_t*>(buffer.data()) + decompressedStartIdx, width * sizeof(T));

            verticalIter.push_back(i);
        }
    }
    {
        PSAPI_PROFILE_SCOPE("DecompressPackBits");
        // Decompress using the PackBits algorithm
        std::for_each(std::execution::par, verticalIter.begin(), verticalIter.end(), [&](auto index)
            {
#ifdef __AVX2__
                RLE_Impl::DecompressPackBitsAVX2<T>(compressedDataSpans[index], decompressedDataSpans[index]);
#else
                RLE_Impl::DecompressPackBits<T>(compressedDataSpans[index], decompressedDataSpans[index]);
#endif
            });
    }
    // Convert decompressed data to native endianness in-place
    endianDecodeBEArray(buffer);
}


PSAPI_NAMESPACE_END