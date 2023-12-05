#pragma once

#include "Macros.h"
#include "AVX2EndianByteSwap.h"

PSAPI_NAMESPACE_BEGIN

constexpr bool is_little_endian = (std::endian::native == std::endian::little);


// Perform an endianDecode operation on a binary array (std::vector) and return
// a vector of the given type. Note that the data input may be modified in-place
// and is therefore no longer valid as a non endian decoded vector afterwards
template<typename T>
std::vector<T> endianDecodeBEBinaryArray(std::vector<uint8_t>& data)
{
    PROFILE_FUNCTION();
    if (data.size() % sizeof(T) != 0)
    {
        PSAPI_LOG_ERROR("endianDecodeBEBinaryArray", "Tried to decode a binary array which is not a multiple of sizeof(T), got size: %i and sizeof T %i",
            data.size(),
            sizeof(T))
    }

    std::vector<T> nativeData;
    nativeData.reserve(data.size() / sizeof(T));

    // TODO this could potentially be done inline
    for (uint64_t i = 0; i < data.size(); i += sizeof(T))
    {
        const uint8_t* byteData = reinterpret_cast<const uint8_t*>(data.data() + i);
        nativeData.push_back(endianDecodeBE<T>(byteData));
    }

    return nativeData;
}


// Specializations for the image depth formats as they are the most performance critical and we use AVX2 to vectorize the instructions
template <>
inline std::vector<uint8_t> endianDecodeBEBinaryArray(std::vector<uint8_t>& data)
{
    return data;
}


template<>
inline std::vector<uint16_t> endianDecodeBEBinaryArray(std::vector<uint8_t>& data)
{
    PROFILE_FUNCTION();
    if (data.size() % sizeof(uint16_t) != 0)
    {
        PSAPI_LOG_ERROR("endianDecodeBEBinaryArray", "Tried to decode a binary array which is not a multiple of sizeof(T), got size: %i and sizeof T %i",
            data.size(),
            sizeof(uint16_t))
    }

    std::vector<uint16_t> nativeData(data.size() / sizeof(uint16_t));

    // We divide by 32 here as AVX2 has 256bit or 32byte wide registers
    size_t numVecs = data.size() / 32;
    size_t remainderVecs = data.size() % 32;

    // Byte swap what we can in place with AVX2 and memcpy it over after the fact
    for (int i = 0; i < numVecs; ++i)
    {
        std::span<uint8_t, 32> vecSpan{ data.data() + i * 32, 32 };
        if constexpr (is_little_endian)
        {
            byteShuffleAVX2_2Wide_LE(vecSpan);
        }
        else
        {
            byteShuffleAVX2_2Wide_BE(vecSpan);
        }
    }

    std::memcpy(reinterpret_cast<void*>(nativeData.data()), reinterpret_cast<void*>(data.data()), numVecs * 32);

    // Decode the remaining items normally
    for (int i = 0; i < remainderVecs; ++i)
    {
        // Use 16 as we have 256bits that can fit 16 uint16_t and multiply index by 2 which is the byte width
        nativeData[numVecs * 16 + i] = endianDecodeBE<uint16_t>(data.data() + numVecs * 32 + i * 2);
    }

    return nativeData;
}


template<>
inline std::vector<float32_t> endianDecodeBEBinaryArray(std::vector<uint8_t>& data)
{
    PROFILE_FUNCTION();
    if (data.size() % sizeof(float32_t) != 0)
    {
        PSAPI_LOG_ERROR("endianDecodeBEBinaryArray", "Tried to decode a binary array which is not a multiple of sizeof(T), got size: %i and sizeof T %i",
            data.size(),
            sizeof(float32_t))
    }

    std::vector<float32_t> nativeData(data.size() / sizeof(float32_t));

    // We divide by 32 here as AVX2 has 256bit or 32byte wide registers
    size_t numVecs = data.size() / 32;
    size_t remainderVecs = data.size() % 32;

    // Byte swap what we can in place with AVX2 and memcpy it over after the fact
    for (int i = 0; i < numVecs; ++i)
    {
        std::span<uint8_t, 32> vecSpan{ data.data() + i * 32, 32 };
        if constexpr (is_little_endian)
        {
            byteShuffleAVX2_4Wide_LE(vecSpan);
        }
        else
        {
            byteShuffleAVX2_4Wide_BE(vecSpan);
        }
    }

    std::memcpy(reinterpret_cast<void*>(nativeData.data()), reinterpret_cast<void*>(data.data()), numVecs * 32);

    // Decode the remaining items normally
    for (int i = 0; i < remainderVecs; ++i)
    {
        // Use 8 as we have 256bits that can fit 8 float32_t and multiply index by 4 which is the byte width
        nativeData[numVecs * 8 + i] = endianDecodeBE<uint16_t>(data.data() + numVecs * 32 + i * 4);
    }

    return nativeData;
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

PSAPI_NAMESPACE_END