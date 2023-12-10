#include "ImageResources.h"

#include "Macros.h"
#include "Enum.h"
#include "Read.h"
#include "Struct/File.h"
#include "Struct/Section.h"
#include "Struct/ResourceBlock.h"
#include "Struct/Signature.h"
#include "Profiling/Perf/Instrumentor.h"

#include <vector>

#include <cstdint>

PSAPI_NAMESPACE_BEGIN

bool ImageResources::read(File& document, const uint64_t offset)
{
	PROFILE_FUNCTION();
	m_Offset = offset;
	document.setOffset(offset);
	m_Size = RoundUpToMultiple<uint32_t>(ReadBinaryData<uint32_t>(document), 2u) + 4u;

	uint32_t toRead = static_cast<uint32_t>(m_Size) - 4u;
	while (toRead > 0)
	{
		ResourceBlock resource;
		resource.read(document);
		toRead -= resource.m_BlockSize;
		m_ResourceBlocks.emplace_back(resource);
	}
	return true;
}

PSAPI_NAMESPACE_END