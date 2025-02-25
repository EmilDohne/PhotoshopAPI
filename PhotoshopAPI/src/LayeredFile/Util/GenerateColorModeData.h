#pragma once

#include "Macros.h"
#include "PhotoshopFile/ColorModeData.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "LayeredFile/LayeredFile.h"

#include <memory>

PSAPI_NAMESPACE_BEGIN


// Generate a ColorModeData section based on the options set by the layeredFile
template <typename T>
ColorModeData generate_colormodedata(LayeredFile<T>& layeredFile)
{
	// We dont actually do any generation of this data here as indexed and duotone colours are not currently supported
	// and the 32-bit file exception gets computed on the write of the function
	return ColorModeData();
}

PSAPI_NAMESPACE_END
