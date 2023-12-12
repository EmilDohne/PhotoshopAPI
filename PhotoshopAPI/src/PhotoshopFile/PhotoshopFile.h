#pragma once

#include "Macros.h"
#include "Struct/File.h"

#include "FileHeader.h"
#include "ColorModeData.h"
#include "ImageResources.h"
#include "LayerAndMaskInformation.h"
#include "ImageData.h"

#include <vector>


PSAPI_NAMESPACE_BEGIN


// A Photoshop File (*.psd or *.psb) parsed into this struct to then later be parsed into the LayeredFile structure.
// This is split into two distinct steps to have a programming interface to parse against rather than the raw file structure
// Any "heavy" data like the Channel Image Data or Image Data section only get read on request rather than on loading to 
// save on memory resources
struct PhotoshopFile
{
	FileHeader m_Header;
	ColorModeData m_ColorModeData;
	ImageResources m_ImageResources;
	LayerAndMaskInformation m_LayerMaskInfo;

	PhotoshopFile() = default;
	PhotoshopFile(FileHeader header, ColorModeData colorModeData, ImageResources imageResources, LayerAndMaskInformation layerMaskInfo);

	bool read(File& document);
	void write(File& document);
};


PSAPI_NAMESPACE_END