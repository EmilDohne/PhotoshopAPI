#pragma once

#include "Macros.h"
#include "Util/Enum.h"
#include "PhotoshopFile/FileHeader.h"
#include "Util/ProgressCallback.h"
#include "Util/Logger.h"

#include <memory>
#include <vector>


PSAPI_NAMESPACE_BEGIN

// Forward declare tagged blocks
struct TaggedBlock;

// A storage container for a collection of Tagged Blocks. The specification doesnt specifically mention tagged blocks being unique but 
// we assume so for retrieving tagged blocks. I.e if you retrieve a tagged block it will return the first instance of it
struct TaggedBlockStorage : public FileSection
{
	TaggedBlockStorage() = default;
	TaggedBlockStorage(std::vector<std::shared_ptr<TaggedBlock>> taggedBlocks);

	// Retrieve the object represented by the specified tagged block. Note, this returns the first instance rather than all instances
	// We assume tagged blocks are unique but this may not always be the case. Returns nullptr if no type is found.
	// Usage: specify the type of tagged block you want to retrieve with the template argument as well as the key.
	template <typename T>
	std::shared_ptr<T> getTaggedBlockView(const Enum::TaggedBlockKey key) const;

	/// Wrapper around the other getTaggedBlockView method to be able to omit the "key" argument
	template <typename T>
	std::shared_ptr<T> getTaggedBlockView() const;

	/// Retrieve a vector of all tagged blocks associated with the given type
	template <typename T>
	std::vector<std::shared_ptr<T>> get_tagged_blocks() const;

	/// Retrieve a vector of all base tagged blocks, excluding any classes derived from TaggedBlock.
	std::vector<std::shared_ptr<TaggedBlock>> get_base_tagged_blocks() const;

	// Read a tagged block into m_TaggedBlocks as well as returning a shared_ptr to it.
	// The shared ptr should be used only to retrieve data, hence its markation as const
	const std::shared_ptr<TaggedBlock> readTaggedBlock(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u);

	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding) const;
private:
	std::vector<std::shared_ptr<TaggedBlock>> m_TaggedBlocks;
};


PSAPI_NAMESPACE_END