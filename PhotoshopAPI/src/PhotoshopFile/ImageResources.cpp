#include "ImageResources.h"

#include "Macros.h"
#include "Enum.h"
#include "FileIO/Read.h"
#include "FileIO/Write.h"
#include "FileIO/Util.h"
#include "Struct/File.h"
#include "Struct/Section.h"
#include "Struct/ResourceBlock.h"
#include "Struct/Signature.h"
#include "Profiling/Perf/Instrumentor.h"

#include <vector>

#include <cstdint>

PSAPI_NAMESPACE_BEGIN



// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
ImageResources::ImageResources(std::vector<ResourceBlock> resourceBlocks)
{
	m_Size = 4u;	// This is the length of the size marker which we include in our size
	for (auto& imageResource : resourceBlocks)
	{
		m_Size += imageResource.m_BlockSize;	// This is the whole block size including all markers
	}
	m_ResourceBlocks = std::move(resourceBlocks);
}


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
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


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
void ImageResources::write(File& document)
{
	m_Offset = document.getOffset();
	m_Size = RoundUpToMultiple<uint32_t>(m_Size - 4u, 2u) + 4u;
	WriteBinaryData<uint32_t>(document, m_Size - 4u);

	for (auto& block : m_ResourceBlocks)
	{
		block.write(document);
	}
}

PSAPI_NAMESPACE_END