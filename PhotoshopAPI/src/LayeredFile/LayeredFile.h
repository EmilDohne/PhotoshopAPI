#pragma once

#include "Macros.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "Layer.h"

PSAPI_NAMESPACE_BEGIN

template <typename T>
struct LayeredFile
{
	// The root layers in the file, they may contain multiple levels of sub-layers
	std::vector<std::shared_ptr<Layer>> m_Layers;

	// Generate a LayeredFile instance from a pointer to a photoshop file, taking ownership of it 
	// and discarding it once we are done with it. This involves transferring from a flat layer hierarchy
	// to a layered file using the lrSectionDivider taggedBlock to identify layer breaks
	LayeredFile(std::unique_ptr<PhotoshopFile> file);
};

PSAPI_NAMESPACE_END