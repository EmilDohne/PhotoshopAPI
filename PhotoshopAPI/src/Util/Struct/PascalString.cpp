#include "PascalString.h"

#include "Macros.h"
#include "Read.h"
#include "Struct/File.h"

#include <string>

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
void PascalString::read(File& document, const uint8_t padding)
{
	uint8_t stringSize = ReadBinaryData<uint8_t>(document);
	m_Size = RoundUpToMultiple<uint8_t>(stringSize + 1u, padding);
	std::vector<uint8_t> stringData = ReadBinaryArray<uint8_t>(document, stringSize);
	m_String = std::string(stringData.begin(), stringData.end());

	// Skip the padding bytes
	document.skip(m_Size - 1u - stringSize);
}

PSAPI_NAMESPACE_END