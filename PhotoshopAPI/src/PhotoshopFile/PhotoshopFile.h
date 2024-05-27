#pragma once

#include "Macros.h"
#include "Core/Struct/File.h"

#include "FileHeader.h"
#include "ColorModeData.h"
#include "ImageResources.h"
#include "LayerAndMaskInformation.h"
#include "ImageData.h"

#include <vector>


PSAPI_NAMESPACE_BEGIN


/// A Photoshop File (*.psd or *.psb) parsed into this struct to then later be parsed into the LayeredFile structure.
/// This is split into two distinct steps to have a programming interface to parse against rather than the raw file structure
struct PhotoshopFile
{
	/// The documents FileHeader holding information such as BitDepth, Size and ColorMode
	FileHeader m_Header;
	/// The ColorModeData defining color mapping for some color and depth types
	ColorModeData m_ColorModeData;
	/// A series of ImageResourceBlocks with additional, document related, information
	ImageResources m_ImageResources;
	/// The Section in which Photoshop stores its layer data, this also contains the image pixels themselves
	LayerAndMaskInformation m_LayerMaskInfo;
	/// This section is for interoperability with different software such as lightroom and stores a merged composite of the 
	/// layer hierarchy. This sections existence is supposed to be toggled by the 'Maximize Compatibility' but there is a bug
	/// which means that often it gets written out either way and photoshop actually also expects this section to be there.
	ImageData m_ImageData{};

	PhotoshopFile() = default;

	/// \brief Initialize a PhotoshopFile struct from the individual sections
	PhotoshopFile(FileHeader header, ColorModeData colorModeData, ImageResources&& imageResources, LayerAndMaskInformation&& layerMaskInfo, ImageData imageData) :
		m_Header(header), m_ColorModeData(colorModeData), m_ImageResources(std::move(imageResources)), m_LayerMaskInfo(std::move(layerMaskInfo)), m_ImageData(imageData) {}

	/// \brief Read and Initialize this struct from a File
	///
	/// \param document the file object to read the data from
	void read(File& document);

	/// \brief Write the PhotoshopFile struct to disk
	///
	/// \param document the file object to write the data to
	void write(File& document);
};


PSAPI_NAMESPACE_END