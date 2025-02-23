#include "UnicodeString.h"

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
UnicodeString::UnicodeString(std::string str, const uint8_t padding)
{
	// We check for empty strings here as simdutf would return 0 on the convert_utf8_to_utf16le which
	// would cause us to mistakenly assume its a broken input string
	if (str.size() == 0)
	{
		std::vector<char> padded_empty(padding / 2, static_cast<char>(0));
		std::vector<char16_t> padded_empty_16(padding / 2, static_cast<char16_t>(0));

		FileSection::size(RoundUpToMultiple<uint32_t>(0 + sizeof(uint32_t), padding));

		m_String = std::string(padded_empty.begin(), padded_empty.end());
		m_UTF16String = std::u16string(padded_empty_16.begin(), padded_empty_16.end());
		return;
	}
	// Calculate the required UTF16-LE size and perform the conversion, storing
	// the data
	size_t expectedUtf16Len = simdutf::utf16_length_from_utf8(str.data(), str.size());
	if (expectedUtf16Len * 2u > std::numeric_limits<uint32_t>::max())
	{
		PSAPI_LOG_ERROR("UnicodeString", "UTF16 string would exceed the maximum size allowed for Photoshop Unicode strings, can at most store uint32_t bytes");
	}

	m_UTF16String.resize(expectedUtf16Len);	// The null character termination is implicit
	if (!simdutf::convert_utf8_to_utf16le(str.data(), str.size(), m_UTF16String.data()))
		PSAPI_LOG_ERROR("UnicodeString", "Invalid UTF8 source string '%s' provided, unable to initialize UnicodeString", str.c_str());

	FileSection::size(RoundUpToMultiple<uint32_t>(static_cast<uint32_t>(expectedUtf16Len) * sizeof(char16_t) + sizeof(uint32_t), padding));
	m_String = str;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint64_t UnicodeString::calculateSize(std::shared_ptr<FileHeader> header /*= nullptr*/) const
{
	// We actually already take care of initializing the size in the constructor therefore it is valid
	return FileSection::size();
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
const std::string UnicodeString::getString() const noexcept
{
	return m_String;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
const std::string_view UnicodeString::getStringView() const noexcept
{
	return m_String;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::u16string UnicodeString::getUTF16String() const noexcept
{
	return m_UTF16String;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::u16string UnicodeString::convertUTF8ToUTF16LE(const std::string& str)
{
	// Calculate the required UTF16-LE size and perform the conversion, storing
	// the data
	std::u16string data{};
	size_t expectedUtf16Len = simdutf::utf16_length_from_utf8(str.data(), str.size());
	data.resize(expectedUtf16Len);
	// We use the property that this function returns 0 if no data was converted to check
	// for correctness
	if (simdutf::convert_utf8_to_utf16le(str.data(), str.size(), data.data()))
		return data;
	PSAPI_LOG_WARNING("UnicodeString", "Invalid UTF8 source string '%s' provided, returning an empty std::u16string", str.c_str());
	return {};
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::u16string UnicodeString::convertUTF8ToUTF16BE(const std::string& str)
{
	// Calculate the required UTF16-BE size and perform the conversion, storing
	// the data
	std::u16string data{};
	size_t expectedUtf16Len = simdutf::utf16_length_from_utf8(str.data(), str.size());
	data.resize(expectedUtf16Len);
	// We use the property that this function returns 0 if no data was converted to check
	// for correctness
	if (simdutf::convert_utf8_to_utf16be(str.data(), str.size(), data.data()))
		return data;
	PSAPI_LOG_WARNING("UnicodeString", "Invalid UTF8 source string '%s' provided, returning an empty std::u16string", str.c_str());
	return {};
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::string UnicodeString::convertUTF16LEtoUTF8(const std::u16string& str)
{
	// Calculate the required UTF8 size and perform the conversion, storing
	// the data
	std::string data{};
	size_t expectedUtf8Len = simdutf::utf8_length_from_utf16le(str.data(), str.size());
	data.resize(expectedUtf8Len);
	// We use the property that this function returns 0 if no data was converted to check
	// for correctness
	if (simdutf::convert_utf16le_to_utf8(str.data(), str.size(), data.data()))
		return data;
	PSAPI_LOG_WARNING("UnicodeString", "Invalid UTF8 source string '%s' provided, returning an empty std::u16string", str.c_str());
	return {};
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::string UnicodeString::convertUTF16BEtoUTF8(const std::u16string& str)
{
	// Calculate the required UTF8 size and perform the conversion, storing
	// the data
	std::string data{};
	size_t expectedUtf8Len = simdutf::utf8_length_from_utf16le(str.data(), str.size());
	data.resize(expectedUtf8Len);
	// We use the property that this function returns 0 if no data was converted to check
	// for correctness
	if (simdutf::convert_utf16be_to_utf8(str.data(), str.size(), data.data()))
		return data;
	PSAPI_LOG_WARNING("UnicodeString", "Invalid UTF8 source string '%s' provided, returning an empty std::u16string", str.c_str());
	return {};
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void UnicodeString::read(File& document, const uint8_t padding)
{
	// The number of code units does not appear to include the two-byte null
	// termination
	uint32_t numCodeUnits = ReadBinaryData<uint32_t>(document);
	uint32_t numBytes = numCodeUnits * 2;

	FileSection::size(RoundUpToMultiple<uint32_t>(numBytes + sizeof(uint32_t), padding));
	// This UTF16 data is now in UTF16LE format (rather than the UTF16BE stored on disk)
	std::vector<char16_t> utf16Data = ReadBinaryArray<char16_t>(document, numBytes);

	m_UTF16String = std::u16string(utf16Data.begin(), utf16Data.end());

	// We check for empty strings here as simdutf would return 0 on the convert_utf8_to_utf16le which
	// would cause us to mistakenly assume its a broken input string
	if (numBytes == 0u)
	{
		m_String = {};
		// Skip the padding bytes (if any)
		document.skip(FileSection::size() - sizeof(uint32_t) - numBytes);
		return;
	}

	// Calculate the required UTF8 size and perform the conversion
	size_t expectedUtf8Len = simdutf::utf8_length_from_utf16le(utf16Data.data(), utf16Data.size());
	m_String.resize(expectedUtf8Len);
	if (!simdutf::convert_utf16le_to_utf8(utf16Data.data(), utf16Data.size(), m_String.data()))
	{
		PSAPI_LOG_ERROR("UnicodeString", "Invalid UnicodeString encountered at file position %zu, unable to parse it", document.getOffset() - numBytes);
	}

	// Skip the padding bytes (if any)
	document.skip(FileSection::size() - sizeof(uint32_t) - numBytes);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void UnicodeString::write(File& document) const
{
	// The length marker only denotes the actual number of code units not counting any padding
	auto utf16strlen = m_UTF16String.size();
	assert(utf16strlen < std::numeric_limits<uint32_t>::max());
	WriteBinaryData<uint32_t>(document, static_cast<uint32_t>(utf16strlen));

	// Write the string data
	std::vector<uint16_t> stringData(m_UTF16String.begin(), m_UTF16String.end());
	WriteBinaryArray<uint16_t>(document, std::move(stringData));

	// Finally, write the padding bytes, excluding the size marker 
	WritePadddingBytes(document, FileSection::size() - m_UTF16String.size() * sizeof(char16_t) - sizeof(uint32_t));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
bool UnicodeString::operator==(const UnicodeString& other) const
{
	return m_UTF16String == other.getUTF16String() && m_String == other.getString();
}

PSAPI_NAMESPACE_END