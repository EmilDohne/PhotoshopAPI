#pragma once

#include "Macros.h"
#include "Util/Profiling/Perf/Instrumentor.h"
#include "EndianByteSwap.h"
// Disable AVX2 at compile time for a scalar variant
#ifdef __AVX2__
#include "AVX2EndianByteSwap.h"
#endif

#include <algorithm>
#include <execution>
#include <vector>
#include <span>
#include <array>
#include <bit>

#include <cstring>


PSAPI_NAMESPACE_BEGIN


constexpr bool is_little_endian = (std::endian::native == std::endian::little);


// Perform an endianDecode operation on a binary array (std::vector) and return a vector of the given type. 
// Note that the data input may be modified in-place and is therefore no longer valid as a non endian decoded vector afterwards
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#ifdef __AVX2__
	template<typename T>
	std::vector<T> endianDecodeBEBinaryArray(std::vector<uint8_t>& data)
	{
		if (data.size() % sizeof(T) != 0)
		{
			PSAPI_LOG_ERROR("Endian", "Cannot decode binary data whose size is not divisible by sizeof(T), got size %d and sizeof(T) = %d", data.size(), sizeof(T));
		}

		PSAPI_PROFILE_FUNCTION();
		// We want to split up the vector into blocks that can easily fit into a L1 cache 
		// that we process in parallel while the remaining data gets processed serially
		// we assume L1 cache size to be >=64KB for most modern processors

		// Additionally, we have to account for AVX2 SIMD size which is 256 bits or 32 bytes
		// i.e. this means we want to split our data in blocks of 32B * 2KB (2048)
		std::vector<T> decodedData(data.size() / sizeof(T));

		const uint32_t cacheSize = 2048 * 32;	// In bytes
		const uint32_t blockSize = 2048;		// In number of vectors
		uint64_t numVecs = data.size() / 32;	// how many 256 wide vectors we have in our data
		uint64_t numBlocks = numVecs / blockSize;	// how many blocks of 2048 vectors we have

		// Calculate the leftover data that we will compute serially
		uint32_t remainderTotal = data.size() % cacheSize;

		// Create spans of each of the cache blocks to decode them in-place
		std::vector<std::span<uint8_t>> cacheTemporary(numBlocks);
		{
			for (uint64_t i = 0; i < numBlocks; ++i)
			{
				auto cacheAddress = data.data() + cacheSize * i;
				std::span<uint8_t> tmpSpan(cacheAddress, cacheSize);
				cacheTemporary[i] = tmpSpan;
			}
		}

		// Iterate all the blocks and byteShuffle them in-place
		std::for_each(std::execution::par, cacheTemporary.begin(), cacheTemporary.end(),
			[](std::span<uint8_t>& cacheSpan)
			{
				for (uint64_t i = 0; i < blockSize; ++i)
				{
					uint8_t* vecMemoryAddress = cacheSpan.data() + i * 32;
					if constexpr (is_little_endian)
					{
						byteShuffleAVX2_LE<T>(vecMemoryAddress);
					}
					else
					{
						byteShuffleAVX2_BE<T>(vecMemoryAddress);
					}
				}
			});


		// Copy the AVX2 decoded data directly into our decodedData
		std::memcpy(reinterpret_cast<uint8_t*>(decodedData.data()), data.data(), static_cast<uint64_t>(numBlocks) * cacheSize);

		// Note that we add by sizeof(T) here as we need to convert per index
		// the remainderIndex is in binary and we must 
		{
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
#else
	template<typename T>
	std::vector<T> endianDecodeBEBinaryArray(std::vector<uint8_t>& data)
	{
		if (data.size() % sizeof(T) != 0)
		{
			PSAPI_LOG_ERROR("Endian", "Cannot decode binary data whose size is not divisible by sizeof(T), got size %d and sizeof(T) = %d", data.size(), sizeof(T));
		}
		PSAPI_PROFILE_FUNCTION();
		std::vector<T> decodedData(data.size() / sizeof(T));
		for (uint64_t i = 0; i < decodedData.size(); ++i)
		{
			decodedData[i] = endianDecodeBE<T>(&data[i * sizeof(T)]);
		}
		return decodedData;
	}
#endif


// Return the data as we do not need to byteswap here
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <>
inline std::vector<uint8_t> endianDecodeBEBinaryArray(std::vector<uint8_t>& data)
{
    return std::move(data);
}


// Perform a endianDecode operation on an array (std::vector) of items in-place using an extremely fast SIMD + Parallelization
// approach. Can decode ~100 million bytes of data in around a millisecond on a Ryzen 9 5950x
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#ifdef __AVX2__
	template<typename T>
	void endianDecodeBEArray(std::vector<T>& data)
	{
		PSAPI_PROFILE_FUNCTION();
		// We want to split up the vector into blocks that can easily fit into a L1 cache 
		// that we process in parallel while the remaining data gets processed serially
		// we assume L1 cache size to be >=64KB for most modern processors

		// Additionally, we have to account for AVX2 SIMD size which is 256 bits or 32 bytes
		// i.e. this means we want to split our data in blocks of 32B * 2KB (2048)
		const uint32_t cacheSize = 2048 * 32 / sizeof(T);
		const uint32_t blockSize = 2048;
		uint64_t numVecs = data.size() * sizeof(T) / 32;
		uint64_t numBlocks = numVecs / blockSize;

		// Calculate the leftover data that we will compute serially
		uint32_t remainderTotal = data.size() % cacheSize;

		// Create spans of each of the cache blocks to decode them in-place
		std::vector<std::span<T>> cacheTemporary(numBlocks);
		{
			for (uint64_t i = 0; i < numBlocks; ++i)
			{
				auto cacheAddress = data.data() + cacheSize * i;
				std::span<T> tmpSpan(cacheAddress, cacheSize);
				cacheTemporary[i] = tmpSpan;
			}
		}

		// Iterate all the blocks and byteShuffle them in-place
		std::for_each(std::execution::par, cacheTemporary.begin(), cacheTemporary.end(),
			[](std::span<T>& cacheSpan)
			{
				for (uint64_t i = 0; i < blockSize; ++i)
				{
					uint8_t* vecMemoryAddress = reinterpret_cast<uint8_t*>(cacheSpan.data()) + i * 32;
					if constexpr (is_little_endian)
					{
						byteShuffleAVX2_LE<T>(vecMemoryAddress);
					}
					else
					{
						byteShuffleAVX2_BE<T>(vecMemoryAddress);
					}
				}
			});

		// Decode the remainder using just a regular endianDecode
		uint64_t remainderIndex = static_cast<uint64_t>(numBlocks) * cacheSize;
		for (uint64_t i = 0; i < remainderTotal; ++i)
		{
			data[remainderIndex + i] = endianDecodeBE<T>(reinterpret_cast<uint8_t*>(&data[remainderIndex + i]));
		}
	}
#else
	template<typename T>
	void endianDecodeBEArray(std::vector<T>& data)
	{
		PSAPI_PROFILE_FUNCTION();
		for (uint64_t i = 0; i < data.size(); ++i)
		{
			data[i] = endianDecodeBE<T>(reinterpret_cast<uint8_t*>(&data[i]));
		}
	}
#endif


// Do nothing as no byteswap is necessary
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <>
inline void endianDecodeBEArray<uint8_t>([[maybe_unused]] std::vector<uint8_t>& data)
{
}


// Perform a endianDecode operation on a std::span of items in-place using an extremely fast SIMD + Parallelization
// approach. Can decode ~100 million bytes of data in around a millisecond on a Ryzen 9 5950x
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#ifdef __AVX2__
template<typename T>
void endianDecodeBEArray(std::span<T> data)
{
	PSAPI_PROFILE_FUNCTION();
	// We want to split up the vector into blocks that can easily fit into a L1 cache 
	// that we process in parallel while the remaining data gets processed serially
	// we assume L1 cache size to be >=64KB for most modern processors

	// Additionally, we have to account for AVX2 SIMD size which is 256 bits or 32 bytes
	// i.e. this means we want to split our data in blocks of 32B * 2KB (2048)
	const uint32_t cacheSize = 2048 * 32 / sizeof(T);
	const uint32_t blockSize = 2048;
	uint64_t numVecs = data.size() * sizeof(T) / 32;
	uint64_t numBlocks = numVecs / blockSize;

	// Calculate the leftover data that we will compute serially
	uint32_t remainderTotal = data.size() % cacheSize;

	// Create spans of each of the cache blocks to decode them in-place
	std::vector<std::span<T>> cacheTemporary(numBlocks);
	{
		for (uint64_t i = 0; i < numBlocks; ++i)
		{
			auto cacheAddress = data.data() + cacheSize * i;
			std::span<T> tmpSpan(cacheAddress, cacheSize);
			cacheTemporary[i] = tmpSpan;
		}
	}

	// Iterate all the blocks and byteShuffle them in-place
	std::for_each(std::execution::par, cacheTemporary.begin(), cacheTemporary.end(),
		[](std::span<T>& cacheSpan)
		{
			for (uint64_t i = 0; i < blockSize; ++i)
			{
				uint8_t* vecMemoryAddress = reinterpret_cast<uint8_t*>(cacheSpan.data()) + i * 32;
				if constexpr (is_little_endian)
				{
					byteShuffleAVX2_LE<T>(vecMemoryAddress);
				}
				else
				{
					byteShuffleAVX2_BE<T>(vecMemoryAddress);
				}
			}
		});

	// Decode the remainder using just a regular endianDecode
	uint64_t remainderIndex = static_cast<uint64_t>(numBlocks) * cacheSize;
	for (uint64_t i = 0; i < remainderTotal; ++i)
	{
		data[remainderIndex + i] = endianDecodeBE<T>(reinterpret_cast<uint8_t*>(&data[remainderIndex + i]));
	}
}
#else
template<typename T>
void endianDecodeBEArray(std::span<T> data)
{
	PSAPI_PROFILE_FUNCTION();
	for (uint64_t i = 0; i < data.size(); ++i)
	{
		data[i] = endianDecodeBE<T>(reinterpret_cast<uint8_t*>(&data[i]));
	}
}
#endif


// Do nothing as no byteswap is necessary
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <>
inline void endianDecodeBEArray<uint8_t>([[maybe_unused]] std::span<uint8_t> data)
{
}



// Perform a endianEncode operation on an array (std::vector) of items in-place using an extremely fast SIMD + Parallelization
// approach. Can decode ~100 million bytes of data in around a millisecond (~95GB/s) on a Ryzen 9 5950x.
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#ifdef __AVX2__
	template<typename T>
	void endianEncodeBEArray(std::span<T> data)
	{
		PSAPI_PROFILE_FUNCTION();
		// We want to split up the vector into blocks that can easily fit into a L1 cache 
		// that we process in parallel while the remaining data gets processed serially
		// we assume L1 cache size to be >=64KB for most modern processors

		// Additionally, we have to account for AVX2 SIMD size which is 256 bits or 32 bytes
		// i.e. this means we want to split our data in blocks of 32B * 2KB (2048)
		const uint32_t cacheSize = 2048 * 32 / sizeof(T);
		const uint32_t blockSize = 2048;
		uint64_t numVecs = data.size() * sizeof(T) / 32;
		uint64_t numBlocks = numVecs / blockSize;

		// Calculate the leftover data that we will compute serially
		uint32_t remainderTotal = data.size() % cacheSize;

		// Create spans of each of the cache blocks to decode them in-place
		std::vector<std::span<T>> cacheTemporary(numBlocks);
		{
			for (uint64_t i = 0; i < numBlocks; ++i)
			{
				auto cacheAddress = data.data() + cacheSize * i;
				std::span<T> tmpSpan(cacheAddress, cacheSize);
				cacheTemporary[i] = tmpSpan;
			}
		}

		// Iterate all the blocks and byteShuffle them in-place
		std::for_each(std::execution::seq, cacheTemporary.begin(), cacheTemporary.end(),
			[](std::span<T>& cacheSpan)
			{
				for (uint64_t i = 0; i < blockSize; ++i)
				{
					uint8_t* vecMemoryAddress = reinterpret_cast<uint8_t*>(cacheSpan.data()) + i * 32;
					if constexpr (is_little_endian)
					{
						byteShuffleAVX2_LE<T>(vecMemoryAddress);
					}
					else
					{
						byteShuffleAVX2_BE<T>(vecMemoryAddress);
					}
				}
			});

		// Decode the remainder using just a regular endianDecode
		uint64_t remainderIndex = static_cast<uint64_t>(numBlocks) * cacheSize;
		for (uint64_t i = 0; i < remainderTotal; ++i)
		{
			data[remainderIndex + i] = endianEncodeBE<T>(data[remainderIndex + i]);
		}
	}
#else
	template<typename T>
	void endianEncodeBEArray(std::span<T> data)
	{
		PSAPI_PROFILE_FUNCTION();
		for (uint64_t i = 0; i < data.size(); ++i)
		{
			data[i] = endianEncodeBE<T>(data[i]);
		}
	}
#endif

// Do nothing as no byteswap is necessary
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <>
inline void endianEncodeBEArray([[maybe_unused]] std::span<uint8_t> data)
{
}


PSAPI_NAMESPACE_END