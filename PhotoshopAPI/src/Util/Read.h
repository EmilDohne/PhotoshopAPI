#pragma once

#include "../Macros.h"

#include "EndianByteSwap.h"
#include "Logger.h"
#include "Struct/File.h"

#include <fstream>

PSAPI_NAMESPACE_BEGIN

// Read a sizeof(T) amount of data from the given filestream
template <typename T>
T ReadBinaryData(File& document)
{
	T data{};
	document.read(reinterpret_cast<char*>(&data), sizeof(T));
	return endianByteSwap(data);
}

// Read a large amount of data into a std::vector for use on large amounts of data,
// assumes the file is already open for reading
template <typename T> 
inline std::vector<T> ReadBinaryArray(File& document, uint64_t size)
{
	return ReadBinaryArrayImpl<T>(document, size);
}

template <typename T>
inline std::vector<T> ReadBinaryArrayImpl(File& document, uint64_t size)
{
	std::vector<T> data(size);
	document.read(reinterpret_cast<char*>(data.data()), size * sizeof(T));

	if (!isLE)
	{
		return data;
	}

	for (const auto& item : data)
	{
		endianByteSwap(item);
	}

	return data;
}

template<>
inline std::vector<unsigned char> ReadBinaryArrayImpl(File& document, uint64_t size)
{
	std::vector<unsigned char> data(size);
	document.read(reinterpret_cast<char*>(data.data()), size);
	return data;
}

template<>
inline std::vector<char> ReadBinaryArrayImpl(File& document, uint64_t size)
{
	std::vector<char> data(size);
	document.read(reinterpret_cast<char*>(data.data()), size);
	return data;
}

template <typename T>
inline T RoundUpToMultiple(T value, T padding)
{
	if (value < 0)
	{
		PSAPI_LOG_ERROR("RoundUpToMultiple", "Cannot round up a negative value, returning 0");
		return NULL;
	}
	return ((value + padding - 1) / padding) * padding;
}

PSAPI_NAMESPACE_END