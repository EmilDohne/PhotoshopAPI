#include "ImageResources.h"

#include "Macros.h"
#include "Enum.h"
#include "Core/FileIO/Read.h"
#include "Core/FileIO/Write.h"
#include "Core/FileIO/Util.h"
#include "Core/Struct/File.h"
#include "Core/Struct/Section.h"
#include "Core/Struct/ResourceBlock.h"
#include "Core/Struct/Signature.h"
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

	for (const auto& resourcePtr : m_ResourceBlocks)
	{
		size += resourcePtr->calculateSize();
	}
	return size;
}


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
ImageResources::ImageResources(std::vector<std::unique_ptr<ResourceBlock>>&& resourceBlocks)
{
	// Resource blocks are usually trivially copyable so we dont really need to worry
	// about moving here
	m_ResourceBlocks = std::move(resourceBlocks);
	FileSection::size(this->calculateSize());
}


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
void ImageResources::read(File& document, const uint64_t offset)
{
	PSAPI_PROFILE_FUNCTION();
	FileSection::initialize(offset, 0u);
	document.setOffset(offset);
	FileSection::size(RoundUpToMultiple<uint32_t>(ReadBinaryData<uint32_t>(document), 2u) + 4u);

	uint32_t toRead = FileSection::size<uint32_t>() - 4u;
	while (toRead > 0)
	{
		// Parse the resource block which will only read blocks we 
		// have a parser for and skip the rest.
		toRead -= parseResourceBlock(document);
	}
}


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
void ImageResources::write(File& document)
{
	FileSection::initialize(document.getOffset(), RoundUpToMultiple<uint32_t>(FileSection::size<uint32_t>(), 2u));
	WriteBinaryData<uint32_t>(document, FileSection::size<uint32_t>() - 4u);

	for (auto& blockPtr : m_ResourceBlocks)
	{
		blockPtr->write(document);
	}
}


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
uint32_t ImageResources::parseResourceBlock(File& document)
{
	uint64_t blockOffset = document.getOffset();

	Signature signature = Signature(ReadBinaryData<uint32_t>(document));
	if (signature != Signature("8BIM"))
	{
		PSAPI_LOG_ERROR("ResourceBlock", "Signature does not match '8BIM', got '%c%c%c%c' instead",
			signature.m_Representation[0],
			signature.m_Representation[1],
			signature.m_Representation[2],
			signature.m_Representation[3]);
	}
	auto uniqueID = Enum::intToImageResource(ReadBinaryData<uint16_t>(document));

	// Add more resources here as we implement more
	if (uniqueID == Enum::ImageResource::ResolutionInfo)
	{
		auto blockPtr = std::make_unique<ResolutionInfoBlock>();
		blockPtr->read(document, blockOffset);
		uint32_t blockSize = blockPtr->size<uint32_t>();
		m_ResourceBlocks.emplace_back(std::move(blockPtr));
		return blockSize;
	}
	else if (uniqueID == Enum::ImageResource::ICCProfile)
	{
		auto blockPtr = std::make_unique<ICCProfileBlock>();
		blockPtr->read(document, blockOffset);
		uint32_t blockSize = blockPtr->size<uint32_t>();
		m_ResourceBlocks.emplace_back(std::move(blockPtr));
		return blockSize;
	}
	else
	{
		// Skip the block
		PascalString name;
		name.read(document, 2u);
		uint32_t dataSize = RoundUpToMultiple(ReadBinaryData<uint32_t>(document), 2u);
		document.skip(dataSize);
		
		// Calculate the size of the block and return it
		uint32_t size = 4u + 2u + name.size<uint32_t>() + 4u + dataSize;
		return size;
	}
}


PSAPI_NAMESPACE_END