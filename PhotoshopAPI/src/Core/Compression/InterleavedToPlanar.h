#pragma once

#include "Util/Profiling/Perf/Instrumentor.h"

#include <vector>
#include <algorithm>
#include <bit>

// If we compile with C++<20 we replace the stdlib implementation with the compatibility
// library
#if (__cplusplus < 202002L)
#include "tcb_span.hpp"
#else
#include <span>
#endif

#ifdef __AVX2__
#include "immintrin.h"
#endif


PSAPI_NAMESPACE_BEGIN

namespace ZIP_Impl
{
    namespace
    {

#ifdef __AVX2__
        // Interleave 32 floats (4 256-bit wide simd registers) at once converting them from interleaved to planar order
        // Storing them in the appropriate byte_* views. There must be at least 128 bytes available in the 
        // interleaved_bytes as well as at least 32 bytes for each of the byte_* spans. 
        // We have a separate offset for indexing into the interleaved_bytes and each of the byte_* views.
        // For reference, interleaved_offset should be 4x byte_offset
        inline void interleavedToPlanar32(
            const std::span<uint8_t> interleavedData,
            std::span<uint8_t> byte0,
            std::span<uint8_t> byte1,
            std::span<uint8_t> byte2,
            std::span<uint8_t> byte3,
            size_t interleavedOffset,
            size_t byteOffset)
        {
            // Since we cant shuffle across lane-boundaries we essentially shuffle the byte order
            // 1234 1234 1234 1234 1234 1234 1234 1234
            // -> 1111 2222 3333 4444 1111 2222 3333 4444
            const static __m256i shuffle_mask = _mm256_set_epi8(
                31, 27, 23, 19, 30, 26, 22, 18,
                29, 25, 21, 17, 28, 24, 20, 16,
                15, 11, 7, 3, 14, 10, 6, 2,
                13, 9, 5, 1, 12, 8, 4, 0
            );

            // We can permute across lane-boundaries which is what we do to move the per-lane sorted bytes to the individual bytes
            const static __m256i permute_mask = _mm256_set_epi32(
                7, 3, 6, 2, 5, 1, 4, 0
            );


            __m256i _interleaved[4];
            _interleaved[0] = _mm256_shuffle_epi8(_mm256_loadu_si256((__m256i*)(interleavedData.data() + interleavedOffset) + 0), shuffle_mask);
            _interleaved[1] = _mm256_shuffle_epi8(_mm256_loadu_si256((__m256i*)(interleavedData.data() + interleavedOffset) + 1), shuffle_mask);
            _interleaved[2] = _mm256_shuffle_epi8(_mm256_loadu_si256((__m256i*)(interleavedData.data() + interleavedOffset) + 2), shuffle_mask);
            _interleaved[3] = _mm256_shuffle_epi8(_mm256_loadu_si256((__m256i*)(interleavedData.data() + interleavedOffset) + 3), shuffle_mask);

            // This is byte 0 and 1 of the original float
            __m256i byte01_0 = _mm256_unpacklo_epi32(_interleaved[0], _interleaved[1]);
            __m256i byte01_1 = _mm256_unpacklo_epi32(_interleaved[2], _interleaved[3]);

            // Unpack and interleave the byte_01 registers after which we permute them to get them back into their original order
            if constexpr (std::endian::native == std::endian::little)
            {
                _mm256_storeu_si256((__m256i*)(byte3.data() + byteOffset), _mm256_permutevar8x32_epi32(_mm256_unpacklo_epi64(byte01_0, byte01_1), permute_mask));
                _mm256_storeu_si256((__m256i*)(byte2.data() + byteOffset), _mm256_permutevar8x32_epi32(_mm256_unpackhi_epi64(byte01_0, byte01_1), permute_mask));
            }
            else
            {
                _mm256_storeu_si256((__m256i*)(byte0.data() + byteOffset), _mm256_permutevar8x32_epi32(_mm256_unpacklo_epi64(byte01_0, byte01_1), permute_mask));
                _mm256_storeu_si256((__m256i*)(byte1.data() + byteOffset), _mm256_permutevar8x32_epi32(_mm256_unpackhi_epi64(byte01_0, byte01_1), permute_mask));
            }

            // This is byte 2 and 3 of the original float
            __m256i byte23_0 = _mm256_unpackhi_epi32(_interleaved[0], _interleaved[1]);
            __m256i byte23_1 = _mm256_unpackhi_epi32(_interleaved[2], _interleaved[3]);

            // Unpack and interleave the byte_23 registers after which we permute them to get them back into their original order
            if constexpr (std::endian::native == std::endian::little)
            {
                _mm256_storeu_si256((__m256i*)(byte1.data() + byteOffset), _mm256_permutevar8x32_epi32(_mm256_unpacklo_epi64(byte23_0, byte23_1), permute_mask));
                _mm256_storeu_si256((__m256i*)(byte0.data() + byteOffset), _mm256_permutevar8x32_epi32(_mm256_unpackhi_epi64(byte23_0, byte23_1), permute_mask));
            }
            else
            {
                _mm256_storeu_si256((__m256i*)(byte2.data() + byteOffset), _mm256_permutevar8x32_epi32(_mm256_unpacklo_epi64(byte23_0, byte23_1), permute_mask));
                _mm256_storeu_si256((__m256i*)(byte3.data() + byteOffset), _mm256_permutevar8x32_epi32(_mm256_unpackhi_epi64(byte23_0, byte23_1), permute_mask));
            }
        }
#endif
    }

    // Go from interleaved byte order in an array of floats to a planar byte order
    // i.e. 1234 1234 1234 1234 -> 1111 2222 3333 4444 for any array that is larger than 0. Also converts to big-endian order 
    // internally if necessary
    // The interleavedData represents a single scanline in an image of float bytes and must therefore be aligned to 
    // 4 bytes as well as being width * sizeof(float32_t) large. We dont do this check inside of the 
    // function itself so be careful passing data into it! The same goes for planarBuffer although this could be larger
    // than interleavedData
    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    inline void interleavedToPlanarFloat(std::span<uint8_t> interleavedData, std::span<uint8_t> planarBuffer, const uint32_t width)
    {
        // Output planar views
        std::span<uint8_t> byte0View(planarBuffer.data() + 0 * width, width);
        std::span<uint8_t> byte1View(planarBuffer.data() + 1 * width, width);
        std::span<uint8_t> byte2View(planarBuffer.data() + 2 * width, width);
        std::span<uint8_t> byte3View(planarBuffer.data() + 3 * width, width);

#ifdef __AVX2__
        uint32_t numVecs = width / 32;   // Number of vectors we can convert using SIMD
        uint32_t numScalar = width % 32; // Number of scalar operations we need to do to "finish" the data

        for (size_t i = 0; i < numVecs; ++i)
        {
            size_t interleavedOffset = i * 32 * sizeof(float32_t);  // Number of 4-byte pairs we have iterated
            size_t byteOffset = i * 32;                             // Number of bytes we have iterated

            interleavedToPlanar32(interleavedData, byte0View, byte1View, byte2View, byte3View, interleavedOffset, byteOffset);
        }
        for (uint32_t i = 0; i < numScalar; ++i)
        {
            size_t offsetInterleaved = static_cast<size_t>(numVecs) * 32 * sizeof(float32_t) + i * sizeof(float32_t);
            size_t offsetPlanar = static_cast<size_t>(numVecs) * 32 + i;
            if constexpr (std::endian::native == std::endian::little)
            {
                byte0View[offsetPlanar] = interleavedData[offsetInterleaved + 3];
                byte1View[offsetPlanar] = interleavedData[offsetInterleaved + 2];
                byte2View[offsetPlanar] = interleavedData[offsetInterleaved + 1];
                byte3View[offsetPlanar] = interleavedData[offsetInterleaved + 0];
            }
            else
            {
                byte0View[offsetPlanar] = interleavedData[offsetInterleaved + 0];
                byte1View[offsetPlanar] = interleavedData[offsetInterleaved + 1];
                byte2View[offsetPlanar] = interleavedData[offsetInterleaved + 2];
                byte3View[offsetPlanar] = interleavedData[offsetInterleaved + 3];
            }
        }
#else
        for (uint32_t i = 0; i < width; ++i)
        {
            size_t offsetInterleaved = i * sizeof(float32_t);
            size_t offsetPlanar = i;
            if constexpr (std::endian::native == std::endian::little)
            {
                byte0View[offsetPlanar] = interleavedData[offsetInterleaved + 3];
                byte1View[offsetPlanar] = interleavedData[offsetInterleaved + 2];
                byte2View[offsetPlanar] = interleavedData[offsetInterleaved + 1];
                byte3View[offsetPlanar] = interleavedData[offsetInterleaved + 0];
            }
            else
            {
                byte0View[offsetPlanar] = interleavedData[offsetInterleaved + 0];
                byte1View[offsetPlanar] = interleavedData[offsetInterleaved + 1];
                byte2View[offsetPlanar] = interleavedData[offsetInterleaved + 2];
                byte3View[offsetPlanar] = interleavedData[offsetInterleaved + 3];
            }
        }
#endif
    }

}

PSAPI_NAMESPACE_END