#include "PascalString.h"

#include "Macros.h"
#include "FileIO/Read.h"
#include "FileIO/Write.h"
#include "FileIO/Util.h"
#include "Struct/File.h"

#include <string>
#include <limits>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

PSAPI_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
PascalString::PascalString(std::string name, const uint8_t padding)
{
	uint8_t stringSize = name.size();
	m_Size = RoundUpToMultiple<uint8_t>(stringSize + 1u, padding);
	m_String = name;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint64_t PascalString::calculateSize(std::optional<FileHeader> header) const
{
	// We actually already take care of initializing the size in the constructor therefore it is valid
	if (m_Size > std::numeric_limits<uint8_t>::max())
	{
		PSAPI_LOG_ERROR("PascalString", "Size of string exceeds the maximum for a uint8_t, expected a max of 255 but got %" PRIu64 " instead.", m_Size)
	}
	return m_Size;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PascalString::read(File& document, const uint8_t padding)
{
	uint8_t stringSize = ReadBinaryData<uint8_t>(document);
	m_Size = RoundUpToMultiple<uint8_t>(stringSize + 1u, padding);
	std::vector<uint8_t> stringData = ReadBinaryArray<uint8_t>(document, stringSize);
	m_String = std::string(stringData.begin(), stringData.end());

	// Skip the padding bytes
	document.skip(m_Size - 1u - stringSize);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void PascalString::write(File& document, const uint8_t padding) const
{
	// We must limit the string size like this as the length marker is only 1 byte and therefore has limited storage capabilities
	if (m_String.size() >  254u - 254u % padding )
	{
		PSAPI_LOG_ERROR("PascalString", "A pascal string can have a maximum length of 253, got %u", m_String.size())
	}
	WriteBinaryData<uint8_t>(document, static_cast<uint8_t>(m_Size));

	// Exclude the null termination char
	std::vector<uint8_t> stringData(m_String.begin(), m_String.end() - 1u);
	WriteBinaryArray<uint8_t>(document, stringData);

	// Finally, write the padding bytes
	for (int i = 0; i < m_Size - m_String.size(); ++i)
	{
		WriteBinaryData<uint8_t>(document, 0u);
	}
}

PSAPI_NAMESPACE_END