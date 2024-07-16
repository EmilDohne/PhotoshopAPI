#pragma once

#include "Macros.h"
#include "Logger.h"
#include "Profiling/Perf/Instrumentor.h"

#include <vector>
#include <algorithm>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

#include <cstring>
#include <limits.h>

PSAPI_NAMESPACE_BEGIN


// Decode Big Endian Data
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------


// Perform a byteswap to go from big endian PS data to system endianness
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<typename T>
T endianDecodeBE(const uint8_t* src)
{
    PSAPI_LOG_ERROR("endianByteSwap", "No Byte Swap defined for the given type");
    return T{};
};


// Specializations adapted from: https://github.com/alipha/cpp/blob/master/endian/endian.hpp
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline uint8_t endianDecodeBE<uint8_t>(const uint8_t* src)
{
    return src[0];
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline int8_t endianDecodeBE<int8_t>(const uint8_t* src)
{
    return static_cast<int8_t>(src[0]);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline char endianDecodeBE<char>(const uint8_t* src)
{
	return static_cast<char>(src[0]);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline uint16_t endianDecodeBE<uint16_t>(const uint8_t* src)
{
    return static_cast<uint16_t>(
        static_cast<uint16_t>(src[0]) << 8
        | static_cast<uint16_t>(src[1])
        );
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline int16_t endianDecodeBE<int16_t>(const uint8_t* src)
{
    return static_cast<int16_t>(endianDecodeBE<uint16_t>(src));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
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


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline int32_t endianDecodeBE<int32_t>(const uint8_t* src)
{
    return static_cast<int32_t>(endianDecodeBE<uint32_t>(src));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
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


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline int64_t endianDecodeBE<int64_t>(const uint8_t* src)
{
    return static_cast<int64_t>(endianDecodeBE<uint64_t>(src));
}


// Floats are stored as IEE754 in Photoshop documents, therefore we decode BE and then just reinterpret the result
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline float32_t endianDecodeBE<float32_t>(const uint8_t* src)
{
    uint32_t val = endianDecodeBE<uint32_t>(src);
    return reinterpret_cast<float32_t&>(val);
}


// Floats are stored as IEE754 in Photoshop documents, therefore we decode BE and then just reinterpret the result
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline float64_t endianDecodeBE<float64_t>(const uint8_t* src)
{
    uint64_t val = endianDecodeBE<uint64_t>(src);
    return reinterpret_cast<float64_t&>(val);
}


// Encode Big Endian Data
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------


// Perform a byteswap to go from system endianness to PS big endian data
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<typename T>
T endianEncodeBE(const T src)
{
    PSAPI_LOG_ERROR("endianByteSwap", "No Byte Swap defined for the given type");
    return T{};
}


// Specializations adapted from: https://github.com/alipha/cpp/blob/master/endian/endian.hpp
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline uint8_t endianEncodeBE<uint8_t>(const uint8_t src)
{
    return src;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline int8_t endianEncodeBE<int8_t>(const int8_t src)
{
    return src;
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline char endianEncodeBE<char>(const char src)
{
	return src;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline uint16_t endianEncodeBE<uint16_t>(const uint16_t src)
{
    uint16_t result = 0u;
    uint8_t* resultPtr = reinterpret_cast<uint8_t*>(&result);

    resultPtr[0] = static_cast<uint8_t>(src >> 8);
    resultPtr[1] = static_cast<uint8_t>(src);

    return result;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline int16_t endianEncodeBE<int16_t>(const int16_t src)
{
    return static_cast<int16_t>(endianEncodeBE<uint16_t>(static_cast<uint16_t>(src)));
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline char16_t endianEncodeBE<char16_t>(const char16_t src)
{
	return static_cast<char16_t>(endianEncodeBE<uint16_t>(static_cast<uint16_t>(src)));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline uint32_t endianEncodeBE<uint32_t>(const uint32_t src)
{
    uint32_t result = 0u;
    uint8_t* resultPtr = reinterpret_cast<uint8_t*>(&result);

    resultPtr[0] = static_cast<uint8_t>(src >> 24);
    resultPtr[1] = static_cast<uint8_t>(src >> 16);
    resultPtr[2] = static_cast<uint8_t>(src >> 8);
    resultPtr[3] = static_cast<uint8_t>(src);

    return result;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline int32_t endianEncodeBE<int32_t>(const int32_t src)
{
    return static_cast<int32_t>(endianEncodeBE<uint32_t>(static_cast<uint32_t>(src)));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline uint64_t endianEncodeBE<uint64_t>(const uint64_t src)
{
    uint64_t result = 0u;
    uint8_t* resultPtr = reinterpret_cast<uint8_t*>(&result);

    resultPtr[0] = static_cast<uint8_t>(src >> 56);
    resultPtr[1] = static_cast<uint8_t>(src >> 48);
    resultPtr[2] = static_cast<uint8_t>(src >> 40);
    resultPtr[3] = static_cast<uint8_t>(src >> 32);
    resultPtr[4] = static_cast<uint8_t>(src >> 24);
    resultPtr[5] = static_cast<uint8_t>(src >> 16);
    resultPtr[6] = static_cast<uint8_t>(src >> 8);
    resultPtr[7] = static_cast<uint8_t>(src);

    return result;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline int64_t endianEncodeBE<int64_t>(const int64_t src)
{
    return static_cast<int64_t>(endianEncodeBE<uint64_t>(static_cast<uint64_t>(src)));
}


// Floats are stored as IEE754 in Photoshop documents, therefore we decode BE and then just reinterpret the result
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline float32_t endianEncodeBE<float32_t>(const float32_t src)
{
    // This is unfortunately a bit awkward but we cant reinterpret_cast to uint32_t to do the byteswap
    uint32_t tmp = 0u;
    float32_t res = 0.0f;

    std::memcpy(&tmp, &src, sizeof(float32_t));
    uint32_t val = endianEncodeBE<uint32_t>(tmp);
	std::memcpy(&res, &val, sizeof(float32_t));

    return res;
}


// Floats are stored as IEE754 in Photoshop documents, therefore we decode BE and then just reinterpret the result
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
inline float64_t endianEncodeBE<float64_t>(const float64_t src)
{
    // This is unfortunately a bit awkward but we cant reinterpret_cast to uint64_t to do the byteswap
	uint64_t tmp = 0u;
	float64_t res = 0.0f;

	std::memcpy(&tmp, &src, sizeof(float64_t));
    uint64_t val = endianEncodeBE<uint64_t>(tmp);
	std::memcpy(&res, &val, sizeof(float64_t));

	return res;
}


PSAPI_NAMESPACE_END