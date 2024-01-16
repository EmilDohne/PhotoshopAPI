#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Struct/File.h"
#include "Struct/Section.h"
#include "Struct/TaggedBlockStorage.h"
#include "FileHeader.h"

#include <vector>
#include <memory>
#include <optional>


PSAPI_NAMESPACE_BEGIN


struct AdditionalLayerInfo : public FileSection
{
	TaggedBlockStorage m_TaggedBlocks;

	AdditionalLayerInfo() = default;
	AdditionalLayerInfo(const AdditionalLayerInfo&) = delete;
	AdditionalLayerInfo(AdditionalLayerInfo&&) = default;
	AdditionalLayerInfo& operator=(const AdditionalLayerInfo&) = delete;
	AdditionalLayerInfo& operator=(AdditionalLayerInfo&&) = default;

	// Note that we do not initialize any variables for FileSection here as that will be handled once we write the file
	AdditionalLayerInfo(TaggedBlockStorage& taggedBlocks) : m_TaggedBlocks(std::move(taggedBlocks)) {};

	uint64_t calculateSize(std::shared_ptr<FileHeader> header = nullptr) const override;

	void read(File& document, const FileHeader& header, const uint64_t offset, const uint64_t maxLength, const uint16_t padding = 1u);
	void write(File& document, const FileHeader& header, const uint16_t padding = 1u) const;

	// Get a tagged block from the key and try to cast it to T. If key cannot be found return nullptr
	template <typename T>
	std::optional<std::shared_ptr<T>> getTaggedBlock(const Enum::TaggedBlockKey key) const
	{
		// If it is a nullptr we return nullopt
		std::shared_ptr<T> taggedBlockPtr = this->m_TaggedBlocks.getTaggedBlockView<T>(key);
		if (taggedBlockPtr)
		{
			return taggedBlockPtr;
		}
		return std::nullopt;
	}
};

PSAPI_NAMESPACE_END