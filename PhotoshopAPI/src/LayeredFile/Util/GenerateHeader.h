#pragma once

#include "Macros.h"
#include "PhotoshopFile/FileHeader.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "LayeredFile/LayeredFile.h"

#include <memory>

PSAPI_NAMESPACE_BEGIN


// Generate a header section based on the options set by the layeredFile
template <typename T>
FileHeader generateHeader(const LayeredFile<T>& layeredFile)
{
	FileHeader header(
		layeredFile.getVersion(),
		layeredFile.getNumChannels(),
		layeredFile.m_Width,
		layeredFile.m_Height,
		layeredFile.m_BitDepth,
		layeredFile.m_ColorMode
	);
	return header;
}



PSAPI_NAMESPACE_END
