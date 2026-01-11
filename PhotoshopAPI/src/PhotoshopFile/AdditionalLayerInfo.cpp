#include "AdditionalLayerInfo.h"
#include "LayerAndMaskInformation.h"
#include "FileHeader.h"
#include "Core/TaggedBlocks/TaggedBlock.h"

#include <memory>
#include <optional>

PSAPI_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void AdditionalLayerInfo::read(File& document, const FileHeader& header, ProgressCallback& callback, const uint64_t offset, const uint64_t maxLength, const uint16_t padding)
{
	FileSection::initialize(offset, 0u);
	document.set_offset(offset);

	int64_t toRead = maxLength;
	while (toRead >= 12u)
	{
		auto start_offset = document.get_offset();
		try
		{
			const std::shared_ptr<TaggedBlock> taggedBlock = m_TaggedBlocks.readTaggedBlock(document, header, callback, padding);
		}
		catch (...)
		{
			PSAPI_LOG_WARNING("AdditionalLayerInfo", "Unknown tagged block encountered. Skipping it.");
		}
		toRead -= document.get_offset() - start_offset;
	}
	if (toRead >= 0)
	{
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