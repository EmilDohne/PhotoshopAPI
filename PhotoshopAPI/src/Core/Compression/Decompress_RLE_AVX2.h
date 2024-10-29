/*
This header is not meant to be directly included since we at compile time include or skip this header in Decompress_RLE.h depending on if we 
detect AVX2 SIMD intrinsics. Including this header in your project will lead to breaking compatibility with non-avx2 platforms.
*/

#pragma once

#include <vector>
#include <cstring>

#include "immintrin.h"

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
    // Decompress the incoming compressed data using AVX2 SIMD intrinsics to speed up decoding.
    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    template<typename T>
    void DecompressPackBitsAVX2(std::span<const uint8_t> compressedData, std::span<uint8_t> decompressedData)
    {
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
                // Repeat the next byte after this n times
                const uint8_t repeatValue = compressedData[i + 1];
                __m256i ymmValue = _mm256_set1_epi8(repeatValue);

                uint8_t remaining = static_cast<uint8_t>(257u - value);

                // Process in chunks of 32 bytes
                for (; remaining >= 32; remaining -= 32)
                {
                    _mm256_storeu_si256((__m256i*)(decompressedData.data() + idx), ymmValue);
                    idx += 32;
                }
                // Process the remaining bytes
                for (int j = 0; j < remaining; ++j)
                {
                    decompressedData[idx] = repeatValue;
                    ++idx;
                }
                ++i;
            }
            else
            {
                uint8_t remaining = value + 1;
                uint8_t read_offset = 0;
                // Header byte indicates the next n bytes are to be read as values
                for (; remaining >= 32; remaining -= 32)
                {
                    __m256i ymmData = _mm256_loadu_si256((__m256i*)(compressedData.data() + i + read_offset + 1));
                    _mm256_storeu_si256((__m256i*)(decompressedData.data() + idx), ymmData);
                    idx += 32;
                    read_offset += 32;
                }
                for (; read_offset <= value; ++read_offset)
                {
                    decompressedData[idx] = compressedData[i + read_offset + 1];
                    ++idx;
                }
                i += value + 1;
            }
            ++i;
        }
    }


    // Decompress the incoming compressed data using AVX2 SIMD intrinsics to speed up decoding.
    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    template<typename T>
    std::vector<uint8_t> DecompressPackBitsAVX2(std::span<const uint8_t> compressedData, const uint32_t width, const uint32_t height)
    {
        PSAPI_PROFILE_FUNCTION();
        std::vector<uint8_t> decompressedData(sizeof(T) * static_cast<uint64_t>(width) * static_cast<uint64_t>(height));

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
                // Repeat the next byte after this n times
                const uint8_t repeatValue = compressedData[i + 1];
                __m256i ymmValue = _mm256_set1_epi8(repeatValue);

                uint8_t remaining = 257 - value;

                // Process in chunks of 32 bytes
                for (; remaining >= 32; remaining -= 32)
                {
                    _mm256_storeu_si256((__m256i*)(decompressedData.data() + idx), ymmValue);
                    idx += 32;
                }
                // Process the remaining bytes
                for (int j = 0; j < remaining; ++j)
                {
                    decompressedData[idx] = repeatValue;
                    ++idx;
                }
                ++i;
            }
            else
            {
                uint8_t remaining = value + 1;
                uint8_t read_offset = 0;
                // Header byte indicates the next n bytes are to be read as values
                for (; remaining >= 32; remaining -= 32)
                {
                    __m256i ymmData = _mm256_loadu_si256((__m256i*)(compressedData.data() + i + read_offset + 1));
                    _mm256_storeu_si256((__m256i*)(decompressedData.data() + idx), ymmData);
                    idx += 32;
                    read_offset += 32;
                }
                for (; read_offset <= value; ++read_offset)
                {
                    decompressedData[idx] = compressedData[i + read_offset + 1];
                    ++idx;
                }
                i += value + 1;
            }
            ++i;
        }

        return decompressedData;
    }
}

PSAPI_NAMESPACE_END