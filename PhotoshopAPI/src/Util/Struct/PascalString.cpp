#include "PascalString.h"

#include "../../Macros.h"
#include "../Read.h"
#include "File.h"

#include <string>

PSAPI_NAMESPACE_BEGIN


PascalString::PascalString(File& document, const uint8_t padding)
{
	m_Size = RoundUpToMultiple<uint8_t>(ReadBinaryData<uint8_t>(document) + 1u, padding);
	std::vector<uint8_t> stringData = ReadBinaryArray<uint8_t>(document, m_Size - 1u);
	m_String = std::string(stringData.begin(), stringData.end());
}

PSAPI_NAMESPACE_END