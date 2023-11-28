#pragma once

#include "Macros.h"
#include "Enum.h"
#include "PhotoshopFile/FileHeader.h"

#include <memory>
#include <vector>


PSAPI_NAMESPACE_BEGIN

// Forward declare tagged blocks
struct TaggedBlock;

// A storage container for a collection of Tagged Blocks. The specification doesnt specifically mention tagged blocks being unique but 
// we assume so for retrieving tagged blocks. I.e if you retrieve a tagged block it will return the first instance of it
struct TaggedBlockStorage
{

	// Retrieve the object represented by the specified tagged block. Note, this returns the first instance rather than all instances
	// We assume tagged blocks are unique but this may not always be the case. Returns nullptr if no type is found
	template <typename T>
	std::shared_ptr<T> getTaggedBlockView(const Enum::TaggedBlockKey key) const;

	// Read a tagged block into m_TaggedBlocks as well as returning a shared_ptr to it.
	// The shared ptr should be used only to retrieve data, hence its markation as const
	const std::shared_ptr<TaggedBlock> readTaggedBlock(File& document, const FileHeader& header, const uint16_t padding = 1u);

private:
	std::vector<std::shared_ptr<TaggedBlock>> m_TaggedBlocks;
};


PSAPI_NAMESPACE_END