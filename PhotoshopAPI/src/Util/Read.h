#pragma once

#include "../Macros.h"

#include "EndianByteSwap.h"
#include "Enum.h"
#include "Logger.h"
#include "Struct/File.h"

#include <fstream>
#include <variant>

PSAPI_NAMESPACE_BEGIN

// Read a sizeof(T) amount of data from the given filestream
template <typename T>
T ReadBinaryData(File& document)
{
	uint8_t data[sizeof(T)]{};
	document.read(reinterpret_cast<char *>(data), sizeof(T));
	return endianDecodeBE<T>(data);
}


// Read a variadic amount of bytes from a document based on whether it is PSD or PSB
// and cast to the wider PSB type
template <typename TPsd, typename TPsb>
std::variant<TPsd, TPsb> ReadBinaryDataVariadic(File& document, const Enum::Version version)
{
	switch (version)
	{
	case Enum::Version::Psd:
		uint8_t dataPsd[sizeof(TPsd)];
		document.read(reinterpret_cast<char*>(dataPsd), sizeof(TPsd));
		return endianDecodeBE<TPsd>(dataPsd);
	case Enum::Version::Psb:
		uint8_t dataPsb[sizeof(TPsb)];
		document.read(reinterpret_cast<char*>(dataPsb), sizeof(TPsb));
		return endianDecodeBE<TPsb>(dataPsb);
	default:
		return static_cast<TPsb>(0);
	}
}


// Figure out, at runtime, how big a variable is depending on the version specified in the file header
template <typename TPsd, typename TPsb>
uint32_t SwapPsdPsb(const Enum::Version version)
{
	switch (version)
	{
	case Enum::Version::Psd:
		return sizeof(TPsd);
	case Enum::Version::Psb:
		return sizeof(TPsb);
	default:
		return 0u;
	}
}

// Extract a value from an std::variant and return the PSB type (usually the widest type)
template <typename TPsd, typename TPsb>
TPsb ExtractWidestValue(std::variant<TPsd, TPsb> variant)
{
	if (sizeof(TPsb) < sizeof(TPsd))
	{
		PSAPI_LOG_WARNING("ExtractWidestValue", "PSD value is wider in size than PSB value, will cast down. Might overflow");
	}

	if (std::holds_alternative<TPsd>(variant))
	{
		return static_cast<TPsb>(std::get<TPsd>(variant));
	}
	return std::get<TPsb>(variant);
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