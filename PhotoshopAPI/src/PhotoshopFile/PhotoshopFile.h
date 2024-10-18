#pragma once

#include "Macros.h"
#include "Core/Struct/File.h"

#include "FileHeader.h"
#include "ColorModeData.h"
#include "ImageResources.h"
#include "LayerAndMaskInformation.h"
#include "ImageData.h"

#include "Util/ProgressCallback.h"

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
	void read(File& document, ProgressCallback& callback);

	/// \brief Write the PhotoshopFile struct to disk with an explicit progress callback
	///
	/// \param document the file object to write the data to
	/// \param callback a callback which will report back the current progress of the write operation
	void write(File& document, ProgressCallback& callback);

	/// \brief Scan the header of the PhotoshopFile and get the appropriate bitdepth
	///
	/// This is a very lightweight function to, at runtime be able to distinguish between different bitdepths
	/// 
	/// \param file The path to the psd or psb file
	/// \returns The bitdepth of the file
	static Enum::BitDepth findBitdepth(std::filesystem::path file);
};


PSAPI_NAMESPACE_END