#include "ImageResources.h"

#include "../Macros.h"
#include "../Util/Enum.h"
#include "../Util/Read.h"
#include "../Util/Struct/File.h"
#include "../Util/Struct/Section.h"
#include "../Util/Struct/ResourceBlock.h"
#include "../Util/Struct/Signature.h"

#include <vector>

#include <cstdint>

PSAPI_NAMESPACE_BEGIN

bool ImageResources::read(File& document, const uint64_t offset)
{
	m_Offset = offset;
	document.setOffset(offset);
	m_Size = RoundUpToMultiple<uint32_t>(ReadBinaryData<uint32_t>(document), 2u) + 4u;

	uint32_t toRead = static_cast<uint32_t>(m_Size) - 4u;
	while (toRead > 0)
	{
		ResourceBlock resource = ResourceBlock(document);
		toRead -= resource.m_BlockSize;
		m_ResourceBlocks.emplace_back(resource);
	}
	return true;
}

PSAPI_NAMESPACE_END