#pragma once

#include "Macros.h"
#include "PhotoshopFile/ImageResources.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "LayeredFile/LayeredFile.h"

#include <memory>

PSAPI_NAMESPACE_BEGIN


// Generate a ColorModeData section based on the options set by the layeredFile
template <typename T>
ImageResources generateImageResources(const LayeredFile<T>& layeredFile)
{
	// Initialize an empty image resource section, this might be interesting later for color management purposes (e.g. 
	// setting an ICC profile on the data or specifying the print resolution
	return ImageResources(std::vector<ResourceBlock>{});
}

PSAPI_NAMESPACE_END
