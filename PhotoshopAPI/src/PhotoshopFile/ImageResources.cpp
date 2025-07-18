#include "ImageResources.h"

#include "Macros.h"
#include "Enum.h"
#include "Core/FileIO/Read.h"
#include "Core/FileIO/Write.h"
#include "Core/FileIO/Util.h"
#include "Core/FileIO/LengthMarkers.h"
#include "Core/FileIO/BytesIO.h"
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
ImageResources::ImageResources(std::vector<std::unique_ptr<ResourceBlock>>&& resourceBlocks)
{
	// Resource blocks are usually trivially copyable so we dont really need to worry
	// about moving here
	m_ResourceBlocks = std::move(resourceBlocks);
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
	Impl::ScopedLengthBlock<uint32_t> len_block(document, 2u);

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

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
size_t ImageResources::get_size(const std::span<const uint8_t> data_span)
{
	auto section_len = RoundUpToMultiple<uint32_t>(bytes_io::read_as_and_swap<uint32_t>(data_span, 0), 2u);
	return section_len + 4u; // Include size marker
}

PSAPI_NAMESPACE_END