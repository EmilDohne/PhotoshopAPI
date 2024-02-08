#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Struct/ImageChannel.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"

#include <vector>
#include <string>

PSAPI_NAMESPACE_BEGIN

/// Structure describing a layer mask (pixel based)
template <typename T> 
struct LayerMask
{
	ImageChannel<T> maskData;
	bool isMaskRelativeToLayer = false;	/// This is primarily for roundtripping and the user shouldnt have to touch this
	bool isDisabled = false;
	uint8_t defaultColor = 255u;
	std::optional<uint8_t> maskDensity;
	std::optional<float64_t> maskFeather;

	LayerMask() = default;
};

/// Base Struct for Layers of all types (Group, Image, [Adjustment], etc.) which includes the minimum to parse a generic layer type
template <typename T>
struct Layer
{
	/// Layer Parameters for initialization of a generic layer type. It provides sensible defaults so only what is needed
	/// needs to be overridden
	struct Params
	{
		/// Optional Layer Mask parameter, if none is specified there is no mask. This image data must have the same size as 
		/// the layer itself
		std::optional<std::vector<T>> layerMask = std::nullopt;
		/// The Layer Name to give to the layer, has a maximum length of 255
		std::string layerName = "";
		/// The Layers Blend Mode, all available blend modes are valid except for 'Passthrough' on non-group layers
		Enum::BlendMode blendMode = Enum::BlendMode::Normal;
		/// The X Center coordinate, 0 indicates that the image is centered around the document, a negative value moves the layer to the left
		int32_t posX = 0;
		/// The Y Center coordinate, 0 indicates that the image is centered around the document, a negative value moves the layer to the top
		int32_t posY = 0;
		/// The width of the layer, this value must be passed explicitly as we do not deduce this from the Image Data itself
		uint32_t width = 0u;
		/// The height of the layer, this value must be passed explicitly as we do not deduce this from the Image Data itself
		uint32_t height = 0u;
		/// The Layer opacity, the value displayed by Photoshop will be this value / 2.55 so 255 corresponds to 100% 
		/// while 128 would correspond to ~50%
		uint8_t opacity = 255u;
		// The compression codec of the layer, it is perfectly valid for each layer to be compressed differently
		Enum::Compression compression = Enum::Compression::ZipPrediction; 
		// The Layers color mode, currently only RGB is supported
		Enum::ColorMode colorMode = Enum::ColorMode::RGB;
	};

	std::string m_LayerName;

	/// A pixel layer mask
	std::optional<LayerMask<T>> m_LayerMask;

	Enum::BlendMode m_BlendMode;

	/// Marks whether or not the layer is visible or not
	bool m_IsVisible; 

	/// 0 - 255 despite the appearance being 0-100 in photoshop
	uint8_t m_Opacity;	

	uint32_t m_Width;

	uint32_t m_Height;

	float m_CenterX;

	float m_CenterY;

	Layer() : m_LayerName(""), m_LayerMask({}), m_BlendMode(Enum::BlendMode::Normal), m_IsVisible(true), m_Opacity(255), m_Width(0u), m_Height(0u), m_CenterX(0u), m_CenterY(0u) {};

	/// \brief Initialize a Layer instance from the internal Photoshop File Format structures.
	///
	/// This constructor is responsible for creating a Layer object based on the information
	/// stored in the provided Photoshop File Format structures. It extracts relevant data
	/// from the LayerRecord, ChannelImageData, and FileHeader to set up the Layer.
	///
	/// \param layerRecord The LayerRecord containing information about the layer.
	/// \param channelImageData The ChannelImageData holding the image data.
	/// \param header The FileHeader providing overall file information.
	Layer(const LayerRecord& layerRecord, ChannelImageData& channelImageData, const FileHeader& header);

	/// \brief Function for creating a PhotoshopFile from the layer.
	///
	/// In the future, the intention is to make this a pure virtual function. However, due to
	/// the presence of multiple miscellaneous layers not yet implemented for the initial release,
	/// this function is provided. It generates a tuple containing LayerRecord and ChannelImageData
	/// based on the specified ColorMode, copying data if required, and using the provided FileHeader.
	///
	/// \param colorMode The desired ColorMode for the PhotoshopFile.
	/// \param doCopy A flag indicating whether to perform a copy of the layer data.
	/// \param header The FileHeader providing overall file information.
	/// \return A tuple containing LayerRecord and ChannelImageData representing the layer in the PhotoshopFile.
	virtual std::tuple<LayerRecord, ChannelImageData> toPhotoshop(Enum::ColorMode colorMode, const bool doCopy, const FileHeader& header);
	
	/// Changes the compression mode of all channels in this layer to the given compression mode
	virtual void setCompression(const Enum::Compression compCode);

	virtual ~Layer() = default;

protected:

	/// Optional argument which specifies in global coordinates where the top left of the layer is to e.g. flip or rotate a layer
	/// currently this is only used for roundtripping, therefore optional. This value must be within the layers bounding box (or no
	/// more than .5 away since it is a double)
	std::optional<double> m_ReferencePointX = std::nullopt;
	/// Optional argument which specifies in global coordinates where the top left of the layer is to e.g. flip or rotate a layer
	/// currently this is only used for roundtripping, therefore optional. This value must be within the layers bounding box (or no
	/// more than .5 away since it is a double)
	std::optional<double> m_ReferencePointY = std::nullopt;

	/// \brief Generates the LayerMaskData struct from the layer mask (if provided).
	///
	/// \return An optional containing LayerMaskData if a layer mask is present; otherwise, std::nullopt.
	std::optional<LayerRecords::LayerMaskData> generateMaskData(const FileHeader& header);

	/// \brief Generate the layer name as a Pascal string.
	///
	/// \return A PascalString representing the layer name.
	PascalString generatePascalString();

	/// \brief Generate the tagged blocks necessary for writing the layer
	virtual std::vector<std::shared_ptr<TaggedBlock>> generateTaggedBlocks();

	/// \brief Generate the layer blending ranges (which for now are just the defaults).
	///
	/// The blending ranges depend on the specified ColorMode. This function returns the default
	/// blending ranges for the given color mode.
	///
	/// \param colorMode The ColorMode to determine the blending ranges.
	/// \return A LayerBlendingRanges object representing the layer blending ranges.
	LayerRecords::LayerBlendingRanges generateBlendingRanges(const Enum::ColorMode colorMode);

	/// \brief Extract the layer mask into a tuple of channel information and image data.
	///
	/// If doCopy is set to false, the mask can be considered invalidated and must no longer be accessed.
	///
	/// \param doCopy A flag indicating whether to perform a copy of the layer mask.
	/// \return An optional containing a tuple of ChannelInformation and a unique_ptr to BaseImageChannel.
	std::optional<std::tuple<LayerRecords::ChannelInformation, std::unique_ptr<BaseImageChannel>>> extractLayerMask(bool doCopy);
};


PSAPI_NAMESPACE_END