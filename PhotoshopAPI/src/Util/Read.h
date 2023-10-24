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
	uint8_t data[sizeof(T)];
	document.read(reinterpret_cast<char *>(data), sizeof(T));
	return endianDecodeBE<T>(data);
}

// Read a large amount of data into a std::vector for use on large amounts of data,
// assumes the file is already open for reading
template <typename T>
inline std::vector<T> ReadBinaryArray(File& document, uint64_t size)
{
	std::vector<T> data(size);
	document.read(reinterpret_cast<char*>(data.data()), size * sizeof(T));
	for (const T item : data)
	{
		endianDecodeBE<T>(reinterpret_cast<uint8_t*>(item));
	}

	return data;
}

template<>
inline std::vector<uint8_t> ReadBinaryArray(File& document, uint64_t size)
{
	std::vector<uint8_t> data(size);
	document.read(reinterpret_cast<char*>(data.data()), size);
	return data;
}

template<>
inline std::vector<int8_t> ReadBinaryArray(File& document, uint64_t size)
{
	std::vector<int8_t> data(size);
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