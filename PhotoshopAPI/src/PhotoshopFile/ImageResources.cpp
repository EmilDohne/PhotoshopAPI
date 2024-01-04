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
uint64_t ImageResources::calculateSize(std::shared_ptr<FileHeader> header /*= nullptr*/) const
{
	uint64_t size = 0u;
	size += 4u;	// Size marker

	for (const auto& resource : m_ResourceBlocks)
	{
		size += resource.calculateSize();
	}
	return size;
}


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
ImageResources::ImageResources(std::vector<ResourceBlock> resourceBlocks)
{
	// Resource blocks are usually trivially copyable so we dont really need to worry
	// about moving here
	m_ResourceBlocks = resourceBlocks;
	m_Size = this->calculateSize();
}


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
void ImageResources::read(File& document, const uint64_t offset)
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
		toRead -= resource.m_Size;
		m_ResourceBlocks.emplace_back(resource);
	}
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