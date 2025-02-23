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

#include "Core/Endian/EndianByteSwap.h"
#include "Core/Endian/EndianByteSwapArr.h"
#include "Util/Enum.h"
#include "Util/Logger.h"
#include "Core/Struct/File.h"
#include "Core/Struct/ByteStream.h"
#include "Util/FileUtil.h"

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
	T val{};
	auto valSpan = Util::toWritableBytes(val);
	document.read(valSpan);
	return endianDecodeBE<T>(valSpan.data());
}


// Read a sizeof(T) amount of data from the given bytestream and decode the data
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename T>
T ReadBinaryData(ByteStream& stream)
{
	T val{};
	auto valSpan = Util::toWritableBytes(val);
	stream.read(valSpan);
	return endianDecodeBE<T>(valSpan.data());
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
	{
		TPsd valPsd{};
		auto dataSpanPsd = Util::toWritableBytes(valPsd);
		document.read(dataSpanPsd);
		return endianDecodeBE<TPsd>(dataSpanPsd.data());
	}
	case Enum::Version::Psb:
	{
		TPsb valPsb{};
		auto dataSpanPsb = Util::toWritableBytes(valPsb);
		document.read(dataSpanPsb);
		return endianDecodeBE<TPsb>(dataSpanPsb.data());
	}
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
	{
		TPsd valPsd{};
		auto dataSpanPsd = Util::toWritableBytes(valPsd);
		stream.read(dataSpanPsd);
		return endianDecodeBE<TPsd>(dataSpanPsd.data());
	}
	case Enum::Version::Psb:
	{
		TPsb valPsb{};
		auto dataSpanPsb = Util::toWritableBytes(valPsb);
		stream.read(dataSpanPsb);
		return endianDecodeBE<TPsb>(dataSpanPsb.data());
	}
	default:
		return static_cast<TPsb>(0);
	}
}


// Read a large amount of data into a std::vector, assumes the file is already open 
// for reading. The size parameter indicates the amount of bytes
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename T>
std::vector<T> ReadBinaryArray(File& document, uint64_t size)
{
	// Check that the data we are trying to read is cleanly divisible as we would trunkate bytes otherwise
	if (size % sizeof(T) != 0)
	{
		PSAPI_LOG_ERROR("ReadBinaryArray", "Was given a binary size of %" PRIu64 " but that is not cleanly divisible by the size of the datatype T, which is %i",
			size, sizeof(T));
	}

	std::vector<T> data(size / sizeof(T));
	document.read(Util::toWritableBytes(data));
	endianEncodeBEArray<T>(data);

	return data;
}


// Read a large amount of data into a std::vector, assumes the file is already open 
// for reading. The size parameter indicates the amount of bytes and the offset 
// parameter indicates where to read from. After the read is complete we set the offset
// back to what it was before
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename T>
std::vector<T> ReadBinaryArray(File& document, uint64_t offset, uint64_t size)
{
	uint64_t initialOffset = document.getOffset();
	document.setOffset(offset);

	// Check that the data we are trying to read is cleanly divisible as we would trunkate bytes otherwise
	if (size % sizeof(T) != 0)
	{
		PSAPI_LOG_ERROR("ReadBinaryArray", "Was given a binary size of %" PRIu64 " but that is not cleanly divisible by the size of the datatype T, which is %i",
			size, sizeof(T));
	}

	std::vector<T> data(size / sizeof(T));
	document.read(Util::toWritableBytes(data));
	endianEncodeBEArray<T>(data);

	document.setOffset(initialOffset);

	return data;
}


// Read a large amount of data into a pre-allocated buffer, assumes the file is already open 
// for reading. The size parameter indicates the amount of bytes and the offset 
// parameter indicates where to read from. After the read is complete we set the offset
// back to what it was before
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename T>
void ReadBinaryArray(File& document, std::span<T> buffer, uint64_t offset, uint64_t size)
{
	uint64_t initialOffset = document.getOffset();
	document.setOffset(offset);

	// Check that the data we are trying to read is cleanly divisible as we would trunkate bytes otherwise
	if (size % sizeof(T) != 0)
	{
		PSAPI_LOG_ERROR("ReadBinaryArray", "Was given a binary size of %" PRIu64 " but that is not cleanly divisible by the size of the datatype T, which is %i",
			size, sizeof(T));
	}
	if (buffer.size() / sizeof(T) != size)
	{
		PSAPI_LOG_ERROR("ReadBinaryArray", "Invalid size parameter passed, expected %zu bytes but instead got %" PRIu64 " bytes", buffer.size() / sizeof(T), size);
	}

	document.read(Util::toWritableBytes(buffer));
	endianEncodeBEArray<T>(buffer);

	document.setOffset(initialOffset);
}




// Read a large amount of data into a std::vector from a bytestream
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename T>
std::vector<T> ReadBinaryArray(ByteStream& stream, uint64_t size)
{
	// Check that the data we are trying to read is cleanly divisible as we would trunkate bytes otherwise
	if (size % sizeof(T) != 0)
	{
		PSAPI_LOG_ERROR("ReadBinaryArray", "Was given a binary size of %" PRIu64 " but that is not cleanly divisible by the size of the datatype T, which is %i",
			size, sizeof(T));
	}

	std::vector<T> data(size / sizeof(T));
	stream.read(Util::toWritableBytes(data));
	for (T item : data)
	{
		endianDecodeBE<T>(reinterpret_cast<uint8_t*>(&item));
	}

	return data;
}


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename T>
std::vector<T> ReadBinaryArray(ByteStream& stream, uint64_t offset, uint64_t size)
{
	// Check that the data we are trying to read is cleanly divisible as we would trunkate bytes otherwise
	if (size % sizeof(T) != 0)
	{
		PSAPI_LOG_ERROR("ReadBinaryArray", "Was given a binary size of %" PRIu64 " but that is not cleanly divisible by the size of the datatype T, which is %i",
			size, sizeof(T));
	}

	std::vector<T> data(size / sizeof(T));
	stream.read(Util::toWritableBytes(data), offset);
	endianDecodeBEArray(data);
	return data;
}


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename T>
void ReadBinaryArray(ByteStream& stream, std::span<T> buffer, uint64_t offset, uint64_t size)
{
	// Check that the data we are trying to read is cleanly divisible as we would trunkate bytes otherwise
	if (size % sizeof(T) != 0)
	{
		PSAPI_LOG_ERROR("ReadBinaryArray", "Was given a binary size of %" PRIu64 " but that is not cleanly divisible by the size of the datatype T, which is %i",
			size, sizeof(T));
	}

	std::vector<T> data(size / sizeof(T));
	stream.read(Util::toWritableBytes(data), offset);
	endianDecodeBEArray<T>(buffer);
}



// Specialization which just reads without endian decoding as it isnt necessary 
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template<>
inline std::vector<uint8_t> ReadBinaryArray(File& document, uint64_t size)
{
	std::vector<uint8_t> data(size);
	document.read(data);
	return data;
}


// Specialization which just reads without endian decoding as it isnt necessary 
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template<>
inline std::vector<int8_t> ReadBinaryArray(File& document, uint64_t size)
{
	std::vector<int8_t> data(size);
	document.read(Util::toWritableBytes(data));
	return data;
}


// Specialization which just reads without endian decoding as it isnt necessary 
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template<>
inline std::vector<uint8_t> ReadBinaryArray(File& document, uint64_t offset, uint64_t size)
{
	uint64_t initialOffset = document.getOffset();
	document.setOffset(offset);
	std::vector<uint8_t> data(size);
	document.read(data);
	document.setOffset(initialOffset);
	return data;
}


// Specialization which just reads without endian decoding as it isnt necessary 
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template<>
inline std::vector<int8_t> ReadBinaryArray(File& document, uint64_t offset, uint64_t size)
{
	uint64_t initialOffset = document.getOffset();
	document.setOffset(offset);
	std::vector<int8_t> data(size);
	document.read(Util::toWritableBytes(data));
	document.setOffset(initialOffset);
	return data;
}


// Specialization which just reads without endian decoding as it isnt necessary 
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template<>
inline std::vector<uint8_t> ReadBinaryArray(ByteStream& stream, uint64_t size)
{
	std::vector<uint8_t> data(size);
	stream.read(data);
	return data;
}


// Specialization which just reads without endian decoding as it isnt necessary 
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template<>
inline std::vector<int8_t> ReadBinaryArray(ByteStream& stream, uint64_t size)
{
	std::vector<int8_t> data(size);
	stream.read(Util::toWritableBytes(data));
	return data;
}


// Specialization which just reads without endian decoding as it isnt necessary 
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template<>
inline std::vector<uint8_t> ReadBinaryArray(ByteStream& stream, uint64_t offset, uint64_t size)
{
	std::vector<uint8_t> data(size);
	stream.read(data, offset);
	return data;
}


// Specialization which just reads without endian decoding as it isnt necessary 
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template<>
inline std::vector<int8_t> ReadBinaryArray(ByteStream& stream, uint64_t offset, uint64_t size)
{
	std::vector<int8_t> data(size);
	stream.read(Util::toWritableBytes(data), offset);
	return data;
}


PSAPI_NAMESPACE_END