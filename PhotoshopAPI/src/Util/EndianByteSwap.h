#pragma once

#include "../Macros.h"
#include "Logger.h"

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include <limits.h>


PSAPI_NAMESPACE_BEGIN


// Perform a byteswap to go from big endian PS data to system endianness
// See https://commandcenter.blogspot.com/2012/04/byte-order-fallacy.html
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
inline uint16_t endianDecodeBE<uint16_t>(const uint8_t* src)
{
    return static_cast<uint16_t>(
        static_cast<uint16_t>(src[0]) << 8
        | static_cast<uint16_t>(src[1])
        );
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


PSAPI_NAMESPACE_END