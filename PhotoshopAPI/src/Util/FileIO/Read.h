/*
A FileIO read interface with convenience functions that access either File or ByteStream structs meant to simplify the reading from files as the conventional 
method involves quite a lot of lines of code as well as casting our data to raw pointers which we neatly wrap around here.

The main functions to read from a file or bytestream are

ReadBinaryData<T>						// Read a given type from the file, handles endian conversions internally
ReadBinaryDataVariadic<TPsd, TPsb>		// Read a variable amount of data from the file depending on version (Psd or Psb)
ReadBinaryArray<T>						// Read a large amount of binary data into a std::vector

*/

#pragma once

#include "Macros.h"

#include "Endian/EndianByteSwap.h"
#include "Enum.h"
#include "Logger.h"
#include "Struct/File.h"
#include "Struct/ByteStream.h"

#include <fstream>
#include <variant>
#include <vector>


PSAPI_NAMESPACE_BEGIN

// Read a sizeof(T) amount of data from the given filestream and decode the data
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename T>
T ReadBinaryData(File& document)
{
	uint8_t data[sizeof(T)]{};
	document.read(reinterpret_cast<char *>(data), sizeof(T));
	return endianDecodeBE<T>(data);
}


// Read a sizeof(T) amount of data from the given bytestream and decode the data
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename T>
T ReadBinaryData(ByteStream& stream)
{
	uint8_t data[sizeof(T)]{};
	stream.read(reinterpret_cast<char*>(data), sizeof(T));
	return endianDecodeBE<T>(data);
}


// Read a variadic amount of bytes from a document based on whether it is PSD or PSB
// and cast to the wider PSB type while decoding the result
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
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


// Read a variadic amount of bytes from a bytestream based on whether it is PSD or PSB
// and cast to the wider PSB type while decoding the result
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename TPsd, typename TPsb>
std::variant<TPsd, TPsb> ReadBinaryDataVariadic(ByteStream& stream, const Enum::Version version)
{
	switch (version)
	{
	case Enum::Version::Psd:
		uint8_t dataPsd[sizeof(TPsd)];
		stream.read(reinterpret_cast<char*>(dataPsd), sizeof(TPsd));
		return endianDecodeBE<TPsd>(dataPsd);
	case Enum::Version::Psb:
		uint8_t dataPsb[sizeof(TPsb)];
		stream.read(reinterpret_cast<char*>(dataPsb), sizeof(TPsb));
		return endianDecodeBE<TPsb>(dataPsb);
	default:
		return static_cast<TPsb>(0);
	}
}


// Read a large amount of data into a std::vector, assumes the file is already open 
// for reading. The size parameter indicates the amount of bytes
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename T>
inline std::vector<T> ReadBinaryArray(File& document, uint64_t size)
{
	// Check that the data we are trying to read is cleanly divisible as we would trunkate bytes otherwise
	if (size % sizeof(T) != 0)
	{
		PSAPI_LOG_ERROR("ReadBinaryArray", "Was given a binary size of %" PRIu64 " but that is not cleanly divisible by the size of the datatype T, which is %i",
			size, sizeof(T))
	}

	std::vector<T> data(size / sizeof(T));
	document.read(reinterpret_cast<char*>(data.data()), size);
	for (T item : data)
	{
		endianDecodeBE<T>(reinterpret_cast<uint8_t*>(&item));
	}

	return data;
}


// Read a large amount of data into a std::vector from a bytestream
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename T>
inline std::vector<T> ReadBinaryArray(ByteStream& stream, uint64_t size)
{
	// Check that the data we are trying to read is cleanly divisible as we would trunkate bytes otherwise
	if (size % sizeof(T) != 0)
	{
		PSAPI_LOG_ERROR("ReadBinaryArray", "Was given a binary size of %" PRIu64 " but that is not cleanly divisible by the size of the datatype T, which is %i",
			size, sizeof(T))
	}

	std::vector<T> data(size / sizeof(T));
	stream.read(reinterpret_cast<char*>(data.data()), size);
	for (T item : data)
	{
		endianDecodeBE<T>(reinterpret_cast<uint8_t*>(&item));
	}

	return data;
}


// TODO this could be sped up using our AVX2 decoder for endian decoding
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename T>
inline std::vector<T> ReadBinaryArray(ByteStream& stream, uint64_t offset, uint64_t size)
{
	// Check that the data we are trying to read is cleanly divisible as we would trunkate bytes otherwise
	if (size % sizeof(T) != 0)
	{
		PSAPI_LOG_ERROR("ReadBinaryArray", "Was given a binary size of %" PRIu64 " but that is not cleanly divisible by the size of the datatype T, which is %i",
			size, sizeof(T))
	}

	std::vector<T> data(size / sizeof(T));
	stream.setOffsetAndRead(reinterpret_cast<char*>(data.data()), offset, size);
	for (T item : data)
	{
		endianDecodeBE<T>(reinterpret_cast<uint8_t*>(&item));
	}

	return data;
}


// Specialization which just reads without endian decoding as it isnt necessary 
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template<>
inline std::vector<uint8_t> ReadBinaryArray(File& document, uint64_t size)
{
	std::vector<uint8_t> data(size);
	document.read(reinterpret_cast<char*>(data.data()), size);
	return data;
}


// Specialization which just reads without endian decoding as it isnt necessary 
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template<>
inline std::vector<int8_t> ReadBinaryArray(File& document, uint64_t size)
{
	std::vector<int8_t> data(size);
	document.read(reinterpret_cast<char*>(data.data()), size);
	return data;
}


// Specialization which just reads without endian decoding as it isnt necessary 
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template<>
inline std::vector<uint8_t> ReadBinaryArray(ByteStream& stream, uint64_t size)
{
	std::vector<uint8_t> data(size);
	stream.read(reinterpret_cast<char*>(data.data()), size);
	return data;
}


// Specialization which just reads without endian decoding as it isnt necessary 
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template<>
inline std::vector<int8_t> ReadBinaryArray(ByteStream& stream, uint64_t size)
{
	std::vector<int8_t> data(size);
	stream.read(reinterpret_cast<char*>(data.data()), size);
	return data;
}


// Specialization which just reads without endian decoding as it isnt necessary 
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template<>
inline std::vector<uint8_t> ReadBinaryArray(ByteStream& stream, uint64_t offset, uint64_t size)
{
	std::vector<uint8_t> data(size);
	stream.setOffsetAndRead(reinterpret_cast<char*>(data.data()), offset, size);
	return data;
}


// Specialization which just reads without endian decoding as it isnt necessary 
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template<>
inline std::vector<int8_t> ReadBinaryArray(ByteStream& stream, uint64_t offset, uint64_t size)
{
	std::vector<int8_t> data(size);
	stream.setOffsetAndRead(reinterpret_cast<char*>(data.data()), offset, size);
	return data;
}


PSAPI_NAMESPACE_END