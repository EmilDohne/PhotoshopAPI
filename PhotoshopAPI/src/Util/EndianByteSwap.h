#pragma once

#include "../Macros.h"
#include "Logger.h"

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include <limits.h>

#if CHAR_BIT != 8
#error A char bit size which isnt 8 is currently unsupported as it would increase complexity
#endif

PSAPI_NAMESPACE_BEGIN

// Check endianness of the system
namespace 
{
    // Assert whether or not the compiled on system is in in big endian or small endian order
    inline bool isLittleEndian()
    {
        uint64_t i = 1;
        uint8_t* c = (uint8_t*)(&i);
        if (*c)
        {
            return true;
        }
        return false;
    }   
}
inline bool isLE = isLittleEndian();


// Perform a byteswap to go from big endian PS data to little endian. If the system is not little endian exit right away
template<typename T>
inline T endianByteSwap(T src)
{
    return endianByteSwapImpl<T>(src);
}

template<typename T>
inline T endianByteSwapImpl(T src)
{
    if (!isLE)
    {
        return src;
    }
    if (sizeof(T) == 1)
    {
        return src;
    }

    PSAPI_LOG_ERROR("endianByteSwap", "No Byte Swap defined for the given type");
    return src;
};

template<>
inline uint16_t endianByteSwapImpl<uint16_t>(uint16_t src)
{
    if (!isLE)
    {
        return src;
    }
    return ((src & 0x00FF) << 8) | ((src & 0xFF00) >> 8);
}

template<>
inline uint32_t endianByteSwapImpl<uint32_t>(uint32_t src)
{
    if (!isLE)
    {
        return src;
    }
    return ((src & 0x000000FF) << 24) |
        ((src & 0x0000FF00) << 8) |
        ((src & 0x00FF0000) >> 8) |
        ((src & 0xFF000000) >> 24);
}

template<>
inline uint64_t endianByteSwapImpl<uint64_t>(uint64_t src)
{
    if (!isLE)
    {
        return src;
    }
    return  ((src & 0x00000000000000FFULL) << 56) |
        ((src & 0x000000000000FF00ULL) << 40) |
        ((src & 0x0000000000FF0000ULL) << 24) |
        ((src & 0x00000000FF000000ULL) << 8) |
        ((src & 0x000000FF00000000ULL) >> 8) |
        ((src & 0x0000FF0000000000ULL) >> 24) |
        ((src & 0x00FF000000000000ULL) >> 40) |
        ((src & 0xFF00000000000000ULL) >> 56);
}


PSAPI_NAMESPACE_END