#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Struct/File.h"
#include "Struct/Section.h"
#include "Struct/ResourceBlock.h"

#include <vector>

#include <cstdint>

PSAPI_NAMESPACE_BEGIN

/// The ImageResources section holds a list of ResourceBlocks giving additional information over the state of the document such as 
/// DPI or Color Space. It additionally also always holds a rather large XML metadata block which we ignore. For a full list 
/// of what is and isnt in the ImageResources section please refer to the Photoshop File Format reference
struct ImageResources : public FileSection
{
	/// We store our ResourceBlocks here, most of them we do not parse as they hold
	/// irrelevant information to keep memory usage low
	std::vector<ResourceBlock> m_ResourceBlocks;

	ImageResources() { m_Size = 4u; };
	ImageResources(std::vector<ResourceBlock> resourceBlocks);

	uint64_t calculateSize(std::shared_ptr<FileHeader> header = nullptr) const override;

	/// Read the ImageResources from disk, any ImageResources without an implementation 
	/// are not parsed but stored as a generic ResourceBlock
	void read(File& document, const uint64_t offset);

	/// Write the ImageResources to disk using the given document
	void write(File& document);
};


PSAPI_NAMESPACE_END