#pragma once

#include "Macros.h"
#include "Util/Enum.h"
#include "Core/Struct/File.h"
#include "Core/Struct/Section.h"
#include "Core/Struct/TaggedBlockStorage.h"
#include "FileHeader.h"
#include "Util/ProgressCallback.h"

#include <vector>
#include <memory>
#include <optional>


PSAPI_NAMESPACE_BEGIN

/// The AdditionalLayerInfo section exists in two different parts of the Photoshop File Format, once at the end of the LayerAndMaskInformation section as well as at the end of 
/// each LayerRecord instance. These sections may be empty
struct AdditionalLayerInfo : public FileSection
{

	/// Our storage container for a vector of tagged blocks
	TaggedBlockStorage m_TaggedBlocks;

	AdditionalLayerInfo() = default;
	AdditionalLayerInfo(const AdditionalLayerInfo&) = delete;
	AdditionalLayerInfo(AdditionalLayerInfo&&) = default;
	AdditionalLayerInfo& operator=(const AdditionalLayerInfo&) = delete;
	AdditionalLayerInfo& operator=(AdditionalLayerInfo&&) = default;

	// Note that we do not initialize any variables for FileSection here as that will be handled once we write the file
	AdditionalLayerInfo(TaggedBlockStorage& taggedBlocks) : m_TaggedBlocks(std::move(taggedBlocks)) {};

	uint64_t calculateSize(std::shared_ptr<FileHeader> header = nullptr) const override;

	/// Read and Initialize this section. Unlike many other sections we do not usually know the exact size but only a max size. 
	/// Therefore we continuously read and verify that we can read another TaggedBlock with the right signature
	void read(File& document, const FileHeader& header, ProgressCallback& callback, const uint64_t offset, const uint64_t maxLength, const uint16_t padding = 1u);

	/// Write all the stored TaggedBlocks to disk
	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u) const;

	/// Get a tagged block from the key and try to cast it to T. If the key cannot be found return nullptr
	template <typename T>
	requires std::is_base_of_v<TaggedBlock, T>
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