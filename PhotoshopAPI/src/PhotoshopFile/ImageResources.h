#pragma once

#include "Macros.h"
#include "Util/Enum.h"
#include "Core/Struct/File.h"
#include "Core/Struct/Section.h"
#include "Core/Struct/ResourceBlock.h"

#include <vector>
#include <type_traits>

#include <cstdint>

PSAPI_NAMESPACE_BEGIN

/// The ImageResources section holds a list of ResourceBlocks giving additional information over the state of the document such as 
/// DPI or Color Space. It additionally also always holds a rather large XML metadata block which we ignore. For a full list 
/// of what is and isnt in the ImageResources section please refer to the Photoshop File Format reference
struct ImageResources : public FileSection
{
	/// We store our ResourceBlocks here, most of them we do not parse as they hold
	/// irrelevant information to keep memory usage low
	std::vector<std::unique_ptr<ResourceBlock>> m_ResourceBlocks;

	ImageResources() { FileSection::size(4u); };
	ImageResources(std::vector<std::unique_ptr<ResourceBlock>>&& resourceBlocks);

	ImageResources(const ImageResources&) = delete;
	ImageResources(ImageResources&&) = default;
	ImageResources& operator=(const ImageResources&) = delete;
	ImageResources& operator=(ImageResources&&) = default;

	/// Read the ImageResources from disk, any ImageResources without an implementation 
	/// are not parsed and skipped
	void read(File& document, const uint64_t offset);

	/// Write the ImageResources to disk using the given document
	void write(File& document);

	/// Retrieve a resource block view as the given template argument using a key as index to the block
	/// 
	/// \return a non owning ptr to the block or nullptr if the resource block is not found
	template <typename T>
	requires std::is_base_of_v<ResourceBlock, T>
	const T* getResourceBlockView(const Enum::ImageResource key) const
	{
		for (const auto& blockPtr : m_ResourceBlocks)
		{
			if (blockPtr->m_UniqueId == key)
			{
				return dynamic_cast<const T*>(blockPtr.get());
			}
		}
		return nullptr;
	}

private:
	/// Parse a singular resource block, if the type is unkown to us we read until the size 
	/// marker and skip it. Otherwise we push back into m_ResourceBlocks.
	/// This function advances the File pointer
	/// 
	/// \return the amount of bytes read (the size of the block)
	uint32_t parseResourceBlock(File& document);
};



PSAPI_NAMESPACE_END