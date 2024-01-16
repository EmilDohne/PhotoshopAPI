#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Struct/ImageChannel.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"

#include <vector>
#include <string>

PSAPI_NAMESPACE_BEGIN

// Structure describing a layer mask (pixel based)
template <typename T> 
struct LayerMask
{
	ImageChannel<T> maskData;
	bool isDisabled = false;
	std::optional<uint8_t> maskDensity;
	std::optional<float64_t> maskFeather;

	LayerMask() = default;
};

/// Base Struct for Layers of all types (Group, Image, [Adjustment], etc.)
/// Includes the bare minimum 
template <typename T>
struct Layer
{
	std::string m_LayerName;
	std::optional<LayerMask<T>> m_LayerMask;
	Enum::BlendMode m_BlendMode;
	bool m_IsVisible; // Is the layer hidden or not?

	uint8_t m_Opacity;	// 0 - 255 despite the appearance being 0-100 in photoshop

	uint32_t m_Width;
	uint32_t m_Height;
	int32_t m_CenterX;
	int32_t m_CenterY;

	Layer() : m_LayerName(""), m_LayerMask({}), m_BlendMode(Enum::BlendMode::Normal), m_IsVisible(true), m_Opacity(255), m_Width(0u), m_Height(0u), m_CenterX(0u), m_CenterY(0u) {};
	Layer(const LayerRecord& layerRecord, ChannelImageData& channelImageData);

	// Define a function for creating a PhotoshopFile from the layer. In the future the intention is to make this a pure virtual function
	// but seeing as there are multiple miscellaneous layers not yet implemented for the initial release we have this function
	virtual std::tuple<LayerRecord, ChannelImageData> toPhotoshop(Enum::ColorMode colorMode, const bool doCopy, const FileHeader& header);
	virtual ~Layer() = default;

protected:

	// Generates the LayerMaskData struct from the layer mask (if provided)
	std::optional<LayerRecords::LayerMaskData> generateMaskData();

	// Generate the channel extents from the width, height and center coordinates using the 
	// headers coordinates as a reference frame returns top, left, bottom, right in that order
	std::tuple<int32_t, int32_t, int32_t, int32_t> generateExtents(const FileHeader& header);

	// Generate the layer name as a pascal string
	PascalString generatePascalString();

	// Generate the layer blending ranges (which for now are just the defaults)
	// We can do this here as in rgb mode the channels are always grey, r, g, b and alpha (?).
	// The same holds true for other color modes where for greyscale e.g. it would be just grey and alpha (?)
	// and cmyk would be grey, cyan, magenta, yellow, key and alpha (?)
	LayerRecords::LayerBlendingRanges generateBlendingRanges(const Enum::ColorMode colorMode);

	// Extract the layer mask into a tuple of channel information as well as the image data. If doCopy is
	// set to false the mask can be considered invalidated and must no longer be accessed.
	std::optional<std::tuple<LayerRecords::ChannelInformation, std::unique_ptr<BaseImageChannel>>> extractLayerMask(bool doCopy);
};

PSAPI_NAMESPACE_END