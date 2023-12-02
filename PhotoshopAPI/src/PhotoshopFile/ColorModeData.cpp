#include "ColorModeData.h"

#include "FileHeader.h"
#include "Macros.h"
#include "Enum.h"
#include "Read.h"
#include "Struct/File.h"
#include "Struct/Section.h"
#include "Profiling/Perf/Instrumentor.h"

#include <vector>

#include <cstdint>

PSAPI_NAMESPACE_BEGIN

bool ColorModeData::read(File& document)
{
	PROFILE_FUNCTION();

	m_Offset = 26;
	document.setOffset(m_Offset);

	m_Size = static_cast<uint64_t>(ReadBinaryData<uint32_t>(document)) + 4u;

	// Just dump the data without parsing it
	if (m_Size > 0)
	{
		m_Data = ReadBinaryArray<uint8_t>(document, m_Size);
	}
	else
	{
		m_Data = std::vector<uint8_t>(0);
	}

	return true;
}

PSAPI_NAMESPACE_END