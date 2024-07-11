#pragma once

#include "Macros.h"
#include "Util.h"
#include "Core/Struct/File.h"
#include "Core/Endian/EndianByteSwap.h"
#include "Core/Endian/EndianByteSwapArr.h"


#if (__cplusplus < 202002L)
#include "tcb_span.hpp"
#else
#include <span>
#endif

#include <variant>
#include <limits>

PSAPI_NAMESPACE_BEGIN


// Write data to a file with the given filestream while encoding the data
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename T>
void WriteBinaryData(File& document, T data)
{
	data = endianEncodeBE<T>(data);
	std::span<uint8_t> dataSpan(reinterpret_cast<uint8_t*>(&data), sizeof(T));
	document.write(dataSpan);
}


// Write a variadic amount of bytes to a document based on whether it is PSD or PSB
// and cast to the wider PSB type while encoding the result
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename TPsd, typename TPsb>
void WriteBinaryDataVariadic(File& document, TPsb data, Enum::Version version)
{
	if (version == Enum::Version::Psd)
	{
		if (data > (std::numeric_limits<TPsd>::max)()) [[unlikely]]
			PSAPI_LOG_ERROR("WriteBinaryDataVariadic", "Value of data exceeds the numeric limits of the max value for type TPsd");
		TPsd psdData = endianEncodeBE<TPsd>(static_cast<TPsd>(data));
		std::span<uint8_t> dataSpan(reinterpret_cast<uint8_t*>(&psdData), sizeof(TPsd));
		document.write(dataSpan);
	}
	else
	{
		TPsb psbData = endianEncodeBE<TPsb>(data);
		std::span<uint8_t> dataSpan(reinterpret_cast<uint8_t*>(&psbData), sizeof(TPsb));
		document.write(dataSpan);
	}
}


// Write an array of data while endian encoding the values
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename T>
void WriteBinaryArray(File& document, std::vector<T>&& data)
{
	// Endian encode in-place
	endianEncodeBEArray<T>(data);
	std::span<uint8_t> dataSpan(reinterpret_cast<uint8_t*>(data.data()), data.size() * sizeof(T));
	document.write(dataSpan);
}


// Write an array of data while endian encoding the values, this will modify the incoming data
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename T>
void WriteBinaryArray(File& document, std::vector<T>& data)
{
	// Endian encode in-place
	endianEncodeBEArray<T>(data);
	std::span<uint8_t> dataSpan(reinterpret_cast<uint8_t*>(data.data()), data.size() * sizeof(T));
	document.write(dataSpan);
}


// Write a given amount of padding bytes with explicit zeroes
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
inline void WritePadddingBytes(File& document, uint64_t numBytes)
{
	if (numBytes == 0) return;
	std::vector<uint8_t> padding(numBytes, 0);
	WriteBinaryArray<uint8_t>(document,std::move(padding));
}


PSAPI_NAMESPACE_END