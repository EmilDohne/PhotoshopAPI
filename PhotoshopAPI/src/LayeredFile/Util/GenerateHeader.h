#pragma once

#include "Macros.h"
#include "PhotoshopFile/FileHeader.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "LayeredFile/LayeredFile.h"

#include <memory>

PSAPI_NAMESPACE_BEGIN


// Generate a header section based on the options set by the layeredFile
template <typename T>
FileHeader generateHeader(LayeredFile<T>& layeredFile)
{
	// Since we decide the version on write we just pass psd in here
	FileHeader header(
		Enum::Version::Psd,
		layeredFile.num_channels(),
		layeredFile.m_Width,
		layeredFile.m_Height,
		layeredFile.m_BitDepth,
		layeredFile.m_ColorMode
	);
	return header;
}



PSAPI_NAMESPACE_END
