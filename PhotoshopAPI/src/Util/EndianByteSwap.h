#pragma once

#include "../Macros.h"
#include "Logger.h"

#include <vector>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include <limits.h>


PSAPI_NAMESPACE_BEGIN


// Perform a endianDecode operation on an array (std::vector) of items
// and return the byteswapped elements
template<typename T>
inline std::vector<T> endianDecodeBEArray(std::vector<T>& data)
{
    for (auto& item : data)
    {
        item = endianDecodeBE<T>(reinterpret_cast<uint8_t*>(&item))
    }
    return std::move(data);
}


// Perform a byteswap to go from big endian PS data to system endianness
template<typename T>
inline T endianDecodeBE(const uint8_t* src)
{
    PSAPI_LOG_ERROR("endianByteSwap", "No Byte Swap defined for the given type");
    return T{};
};

// Specializations adapted from: https://github.com/alipha/cpp/blob/master/endian/endian.hpp
template<>
inline uint8_t endianDecodeBE<uint8_t>(const uint8_t* src)
{
    return src[0];
}

template<>
inline int8_t endianDecodeBE<int8_t>(const uint8_t* src)
{
    return static_cast<int8_t>(src[0]);
}

template<>
inline uint16_t endianDecodeBE<uint16_t>(const uint8_t* src)
{
    return static_cast<uint16_t>(
        static_cast<uint16_t>(src[0]) << 8
        | static_cast<uint16_t>(src[1])
        );
}

template<>
inline int16_t endianDecodeBE<int16_t>(const uint8_t* src)
{
    return static_cast<int16_t>(endianDecodeBE<uint16_t>(src));
}

template<>
inline uint32_t endianDecodeBE<uint32_t>(const uint8_t* src)
{
    return static_cast<uint32_t>(
        static_cast<uint32_t>(src[0]) << 24
        | static_cast<uint32_t>(src[1]) << 16
        | static_cast<uint32_t>(src[2]) << 8
        | static_cast<uint32_t>(src[3])
        );
}

template<>
inline int32_t endianDecodeBE<int32_t>(const uint8_t* src)
{
    return static_cast<int32_t>(endianDecodeBE<uint32_t>(src));
}

template<>
inline uint64_t endianDecodeBE<uint64_t>(const uint8_t* src)
{
    return static_cast<uint64_t>(
        static_cast<uint64_t>(src[0]) << 56
        | static_cast<uint64_t>(src[1]) << 48
        | static_cast<uint64_t>(src[2]) << 40
        | static_cast<uint64_t>(src[3]) << 32
        | static_cast<uint64_t>(src[4]) << 24
        | static_cast<uint64_t>(src[5]) << 16
        | static_cast<uint64_t>(src[6]) << 8
        | static_cast<uint64_t>(src[7])
        );
}

template<>
inline int64_t endianDecodeBE<int64_t>(const uint8_t* src)
{
    return static_cast<int64_t>(endianDecodeBE<uint64_t>(src));
}

// Floats are stored as IEE754 in Photoshop documents, therefore we decode BE and then just reinterpret the result
template<>
inline float32_t endianDecodeBE<float32_t>(const uint8_t* src)
{
    uint32_t val = endianDecodeBE<uint32_t>(src);
    return reinterpret_cast<float32_t&>(val);
}

template<>
inline float64_t endianDecodeBE<float64_t>(const uint8_t* src)
{
    uint64_t val = endianDecodeBE<uint64_t>(src);
    return reinterpret_cast<float64_t&>(val);
}


PSAPI_NAMESPACE_END