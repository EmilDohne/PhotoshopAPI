#pragma once

#include "Macros.h"
#include "Logger.h"

#include <immintrin.h>

PSAPI_NAMESPACE_BEGIN

// Perform a byteshuffle on 2byte wide types, modifies the input span in place.
// This function requires the data span to be 32 wide (256 bit register / 8 bits).
// Please perform checks on whether a byte shuffle is even required before using
// this function
inline void byteShuffleAVX2_2Wide_LE(uint8_t* data)
{
	// Load our span into a 256 wide register
	__m256i vec = _mm256_loadu_si256(reinterpret_cast<__m256i*>(&data[0]));
	
	// Shuffle with a byte mask, note the descending order here
	vec = _mm256_shuffle_epi8(vec, _mm256_set_epi8(
		30, 31, 28, 29, 26, 27, 24, 25,
		22, 23, 20, 21, 18, 19, 16, 17,
		14, 15, 12, 13, 10, 11, 8, 9,
		6, 7, 4, 5, 2, 3, 0, 1
	));

	// Store the result back into the span
	_mm256_storeu_si256(reinterpret_cast<__m256i*>(&data[0]), vec);
}


inline void byteShuffleAVX2_2Wide_BE(uint8_t* data)
{
	// No need to do anything for big endian systems
}


// Perform a byteshuffle on 4byte wide types, modifies the input span in place
// This function requires the data span to be 32 wide (256 bit register / 8 bits)
// Please perform checks on whether a byte shuffle is even required before using
// this function
inline void byteShuffleAVX2_4Wide_LE(uint8_t* data)
{
	// Load our span into a 256 wide register
	__m256i vec = _mm256_loadu_si256(reinterpret_cast<__m256i*>(&data[0]));

	// Shuffle with a byte mask
	vec = _mm256_shuffle_epi8(vec, _mm256_set_epi8(
		28, 29, 30, 31, 24, 25, 26, 27,
		20, 21, 22, 23, 16, 17, 18, 19,
		12, 13, 14, 15, 8, 9, 10, 11,
		4, 5, 6, 7, 0, 1, 2, 3
	));

	// Store the result back into the span
	_mm256_storeu_si256(reinterpret_cast<__m256i*>(&data[0]), vec);
}


inline void byteShuffleAVX2_4Wide_BE(uint8_t* data)
{
	// No need to do anything for big endian systems
}


PSAPI_NAMESPACE_END