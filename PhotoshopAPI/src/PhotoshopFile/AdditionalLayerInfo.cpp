#include "AdditionalLayerInfo.h"
#include "LayerAndMaskInformation.h"
#include "FileHeader.h"
#include "Core/TaggedBlocks/TaggedBlock.h"

#include <memory>
#include <optional>

PSAPI_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
uint64_t AdditionalLayerInfo::calculateSize(std::shared_ptr<FileHeader> header /*= nullptr*/) const
{	
	uint64_t size = m_TaggedBlocks.calculateSize();
	return size;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void AdditionalLayerInfo::read(File& document, const FileHeader& header, ProgressCallback& callback, const uint64_t offset, const uint64_t maxLength, const uint16_t padding)
{
	FileSection::initialize(offset, 0u);
	document.setOffset(offset);

	int64_t toRead = maxLength;
	while (toRead >= 12u)
	{
		const std::shared_ptr<TaggedBlock> taggedBlock = m_TaggedBlocks.readTaggedBlock(document, header, callback, padding);
		toRead -= taggedBlock->totalSize();
		FileSection::size(FileSection::size() + taggedBlock->totalSize());
	}
	if (toRead >= 0)
	{
		FileSection::size(FileSection::size() + toRead);
		document.skip(toRead);
		return;
	}
	PSAPI_LOG_WARNING("AdditionalLayerInfo", "Read too much data for the additional layer info, was allowed %" PRIu64 " but read %" PRIu64 " instead",
		maxLength, maxLength - toRead);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void AdditionalLayerInfo::write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding /*= 1u*/) const
{
	m_TaggedBlocks.write(document, header, callback, padding);
}


PSAPI_NAMESPACE_END