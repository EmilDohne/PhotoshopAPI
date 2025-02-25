#pragma once

#include "Macros.h"
#include "PhotoshopFile/FileHeader.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "LayeredFile/LayeredFile.h"

#include <memory>

PSAPI_NAMESPACE_BEGIN


// Generate a header section based on the options set by the layeredFile
template <typename T>
FileHeader generate_header(LayeredFile<T>& layeredFile)
{
	// Since we decide the version on write we just pass psd in here
	FileHeader header(
		Enum::Version::Psd,
		layeredFile.num_channels(),
		layeredFile.width(),
		layeredFile.height(),
		layeredFile.bitdepth(),
		layeredFile.colormode()
	);
	return header;
}



PSAPI_NAMESPACE_END
