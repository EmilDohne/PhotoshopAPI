#include "PascalString.h"

#include "Macros.h"
#include "Core/FileIO/Read.h"
#include "Core/FileIO/Write.h"
#include "Core/FileIO/Util.h"
#include "Core/Struct/File.h"

#include <string>
#include <limits>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

PSAPI_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
PascalString::PascalString(std::string name, const uint8_t padding)
{
	// We must limit the string size like this as the length marker is only 1 byte and therefore has limited storage capabilities. Since we write
	// out the Unicode Layer name for layers anyways this isnt too bothersome
	std::string truncatedName = name;
	if (truncatedName.size() > 254u - 254u % padding)
	{
		PSAPI_LOG_WARNING("PascalString", "A pascal string can have a maximum length of 254, got %u. Truncating to fit", m_String.size());
		truncatedName = name.substr(0, 254 - 254 % padding);
	}

	uint8_t stringSize = static_cast<uint8_t>(truncatedName.size());
	FileSection::size(RoundUpToMultiple<uint8_t>(stringSize + 1u, padding));
	m_String = truncatedName;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::string PascalString::getString() const noexcept
{
	return m_String;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
const std::string_view PascalString::getStringView() const noexcept
{
	return m_String;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::string PascalString::readString(File& document, const uint8_t padding) noexcept
{
	PascalString str;
	str.read(document, padding);
	return str.getString();
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PascalString::read(File& document, const uint8_t padding) noexcept
{
	uint8_t stringSize = ReadBinaryData<uint8_t>(document);
	FileSection::size(RoundUpToMultiple<uint8_t>(stringSize + 1u, padding));
	std::vector<uint8_t> stringData = ReadBinaryArray<uint8_t>(document, stringSize);
	auto PascalString = std::string(stringData.begin(), stringData.end());
	m_String = convertStrToUTF8(EncodingType::Windows_1252, PascalString);

	// Skip the padding bytes
	document.skip(FileSection::size() - 1u - stringSize);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PascalString::write(File& document) const
{
	if (FileSection::size() == 0)
	{
		PSAPI_LOG_ERROR("PascalString", "Size field is 0 which is not allowed since it will always be at least 1, was the PascalString initialized correctly?");
	}
	std::string nativeStr = ConvertUTF8ToStr(EncodingType::Windows_1252, m_String);

	// The length marker only denotes the actual length of the data, not any padding
	WriteBinaryData<uint8_t>(document, static_cast<uint8_t>(nativeStr.size()));

	std::vector<uint8_t> stringData(nativeStr.begin(), nativeStr.end());
	WriteBinaryArray<uint8_t>(document, std::move(stringData));

	// Finally, write the padding bytes, excluding the size marker
	// Since we store padding on creation we dont need to worry about passing it here
	WritePadddingBytes(document, FileSection::size() - nativeStr.size() - 1u);
}



PSAPI_NAMESPACE_END