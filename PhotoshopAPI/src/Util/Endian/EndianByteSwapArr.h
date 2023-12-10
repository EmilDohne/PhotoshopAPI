#pragma once

#include "Macros.h"
#include "AVX2EndianByteSwap.h"

#include <algorithm>
#include <execution>
#include <vector>
#include <span>
#include <array>
#include <bit>


PSAPI_NAMESPACE_BEGIN

constexpr bool is_little_endian = (std::endian::native == std::endian::little);


// Perform an endianDecode operation on a binary array (std::vector) and return
// a vector of the given type. Note that the data input may be modified in-place
// and is therefore no longer valid as a non endian decoded vector afterwards
template<typename T>
std::vector<T> endianDecodeBEBinaryArray(std::vector<uint8_t>& data)
{
    PROFILE_FUNCTION();
    // We want to split up the vector into blocks that can easily fit into a L1 cache 
    // that we process in parallel while the remaining data gets processed serially
    // we assume L1 cache size to be >=64KB for most modern processors

    // Additionally, we have to account for AVX2 SIMD size which is 256 bits or 32 bytes
    // i.e. this means we want to split our data in blocks of 32B * 2KB (2048)
    std::vector<T> decodedData(data.size() / sizeof(T));

    const uint32_t cacheSize = 2048 * 32;
    const uint32_t blockSize = 2048;
    uint32_t numVecs = data.size() / 32;
    uint32_t numBlocks = numVecs / blockSize;

    // Calculate the leftover data that we will compute serially
    uint32_t remainderTotal = data.size() % cacheSize;


    // Copy each cache member to the individual items
    std::vector<std::array<uint8_t, cacheSize>> cacheTemporary(numBlocks);
    {
        PROFILE_SCOPE("EndianDecodeMemcpy");
        for (uint32_t i = 0; i < numBlocks; ++i)
        {
            void* cacheAddress = data.data() + cacheSize * i;
            std::memcpy(cacheTemporary[i].data(), cacheAddress, cacheSize);
        }
    }

    {
        PROFILE_SCOPE("ForEachEndianDecode");
        // Iterate all the blocks and byteShuffle them in-place
        std::for_each(std::execution::par, cacheTemporary.begin(), cacheTemporary.end(),
            [](std::array<uint8_t, cacheSize>& cacheArray)
            {
                for (uint32_t i = 0; i < blockSize; ++i)
                {
                    std::span<uint8_t, 32> vecSpan{ cacheArray.data() + i * 32, 32 };
                    if constexpr (is_little_endian)
                    {
                        byteShuffleAVX2_2Wide_LE(vecSpan.data());
                    }
                    else
                    {
                        byteShuffleAVX2_2Wide_BE(vecSpan.data());
                    }
                }
            });
    }

    // Copy the AVX2 decoded data directly into our decodedData
    std::memcpy(decodedData.data(), cacheTemporary.data(), static_cast<uint64_t>(numBlocks) * cacheSize);

    // Note that we add by sizeof(uint16_t) here as we need to convert per index
    // the remainderIndex is in binary and we must 
    {
        PROFILE_SCOPE("EndianDecodeSimple");
        uint64_t remainderIndex = static_cast<uint64_t>(numBlocks) * cacheSize;
        for (uint32_t i = 0; i < remainderTotal; i += sizeof(T))
        {
            const uint8_t* memAddress = data.data() + remainderIndex + i;
            // remainderIndex as well as i are both for uint8_t and we need half of that
            const uint64_t decodedDataIndex = (remainderIndex + i) / sizeof(T);
            decodedData[decodedDataIndex] = endianDecodeBE<T>(memAddress);
        }
    }

    return decodedData;
}


template <>
inline std::vector<uint8_t> endianDecodeBEBinaryArray(std::vector<uint8_t>& data)
{
    return data;
}


// Perform a endianDecode operation on an array (std::vector) of items in-place
template<typename T>
void endianDecodeBEArray(std::vector<T>& data)
{
    for (auto& item : data)
    {
        item = endianDecodeBE<T>(reinterpret_cast<uint8_t*>(&item));
    }
}


template<>
inline void endianDecodeBEArray(std::vector<uint16_t>& data)
{
    PROFILE_FUNCTION();
    // We want to split up the vector into blocks that can easily fit into a L1 cache 
    // that we process in parallel while the remaining data gets processed serially
    // we assume L1 cache size to be >=64KB for most modern processors

    // Additionally, we have to account for AVX2 SIMD size which is 256 bits or 32 bytes
    // i.e. this means we want to split our data in blocks of 32B * 2KB (2048)
    // TODO reset this to 2048
    const uint32_t cacheSize = 2048 * 32 / sizeof(uint16_t);
    const uint32_t blockSize = 2048;
    uint32_t numVecs = data.size() / 32 / sizeof(uint16_t);
    uint32_t numBlocks = numVecs / blockSize;

    // Calculate the leftover data that we will compute serially
    uint32_t remainderTotal = data.size() % cacheSize;

    // Create spans of each of the cache blocks to decode them in-place
    std::vector<std::span<uint16_t>> cacheTemporary(numBlocks);
    {
        for (uint32_t i = 0; i < numBlocks; ++i)
        {
            auto cacheAddress = data.data() + cacheSize * i;
            std::span<uint16_t> tmpSpan(cacheAddress, cacheSize);
            cacheTemporary[i] = tmpSpan;
        }
    }

    // Iterate all the blocks and byteShuffle them in-place
    std::for_each(std::execution::par, cacheTemporary.begin(), cacheTemporary.end(),
        [](std::span<uint16_t>& cacheSpan)
        {
            for (uint32_t i = 0; i < blockSize; ++i)
            {
                uint8_t* vecMemoryAddress = reinterpret_cast<uint8_t*>(cacheSpan.data()) + i * 32;
                if constexpr (is_little_endian)
                {
                    byteShuffleAVX2_2Wide_LE(vecMemoryAddress);
                }
                else
                {
                    byteShuffleAVX2_2Wide_BE(vecMemoryAddress);
                }
            }
        });

    // Decode the remainder using just a regular endianDecode
    uint64_t remainderIndex = static_cast<uint64_t>(numBlocks) * cacheSize;
    for (uint64_t i = 0; i < remainderTotal; ++i)
    {
        const uint8_t* memAddress = reinterpret_cast<uint8_t*>(data.data() + remainderIndex + i);
        data[remainderIndex + i] = endianDecodeBE<uint16_t>(memAddress);
    }
}


PSAPI_NAMESPACE_END