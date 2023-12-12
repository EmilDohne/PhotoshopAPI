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
	document.write(reinterpret_cast<char*>(&data), sizeof(T));
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
		document.write(reinterpret_cast<char*>(&psdData), sizeof(TPsd));
	}
	else if (std::holds_alternative<TPsb>(data))
	{
		TPsb psbData = endianEncodeBE<TPsb>(std::get<TPsb>(data));
		document.write(reinterpret_cast<char*>(&psbData), sizeof(TPsb));
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
	document.write(reinterpret_cast<char*>(data.data()), data.size() * sizeof(T));
}


PSAPI_NAMESPACE_END