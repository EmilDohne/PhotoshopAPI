#pragma once

#include "Macros.h"
#include "Util.h"
#include "Struct/File.h"
#include "Endian/EndianByteSwap.h"
#include "Endian/EndianByteSwapArr.h"

#include <span>
#include <variant>

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
void WriteBinaryDataVariadic(File& document, std::variant<TPsd, TPsb> data, Enum::Version version)
{
	if (std::holds_alternative<TPsd>(data))
	{
		TPsd psdData = endianEncodeBE<TPsd>(std::get<TPsd>(data));
		std::span<uint8_t> dataSpan(reinterpret_cast<uint8_t*>(&psdData), sizeof(TPsd));
		document.write(dataSpan);
	}
	else if (std::holds_alternative<TPsb>(data))
	{
		TPsb psbData = endianEncodeBE<TPsb>(std::get<TPsb>(data));
		std::span<uint8_t> dataSpan(reinterpret_cast<uint8_t*>(&psbData), sizeof(TPsb));
		document.write(dataSpan);
	}
}


// Write an array of data while endian encoding the values
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename T>
void WriteBinaryArray(File& document, std::vector<T> data)
{
	// Endian encode in-place
	data = endianEncodeBEArray<T>(data);
	std::span<uint8_t> dataSpan(reinterpret_cast<uint8_t*>(data.data()), data.size() * sizeof(T));
	document.write(dataSpan);
}


PSAPI_NAMESPACE_END