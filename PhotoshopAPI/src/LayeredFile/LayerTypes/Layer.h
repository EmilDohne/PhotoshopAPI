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
	bool isDisabled;
	std::optional<uint8_t> maskDensity;
	std::optional<float64_t> maskFeather;
};

/// Base Struct for Layers of all types (Group, Image, [Adjustment], etc.)
/// Includes the bare minimum 
template <typename T>
struct Layer
{
	std::string m_LayerName;
	std::optional<LayerMask<T>> m_LayerMask;
	Enum::BlendMode m_BlendMode;

	uint8_t m_Opacity;	// 0 - 255 despite the appearance being 0-100 in photoshop

	uint32_t m_Width;
	uint32_t m_Height;
	int32_t m_CenterX;
	int32_t m_CenterY;

	Layer() : m_LayerName(""), m_LayerMask({}), m_BlendMode(Enum::BlendMode::Normal), m_Opacity(255), m_Width(0u), m_Height(0u), m_CenterX(0u), m_CenterY(0u) {};
	Layer(const LayerRecord& layerRecord, const ChannelImageData& channelImageData);

	// Each layer must implement a function that parses it to a global namespace
	virtual std::tuple<LayerRecord, ChannelImageData> toPhotoshop(Enum::ColorMode colorMode) = 0;
	virtual ~Layer() = default;
protected:

	// Generates the LayerMaskData struct from the layer mask (if provided)
	std::optional<LayerRecords::LayerMaskData> generateMaskData();

	// Generate the channel extents from the width, height and center coordinates
	// returns top, left, bottom, right in that order
	std::tuple<int32_t, int32_t, int32_t, int32_t> generateExtents();

	// Generate the layer name as a pascal string
	PascalString generatePascalString();

	// Generate the layer blending ranges (which for now are just the defaults)
	// We can do this here as in rgb mode the channels are always grey, r, g, b and alpha (?).
	// The same holds true for other color modes where for greyscale e.g. it would be just grey and alpha (?)
	// and cmyk would be grey, cyan, magenta, yellow, key and alpha (?)
	LayerRecords::LayerBlendingRanges generateBlendingRanges(const Enum::ColorMode colorMode);
};

PSAPI_NAMESPACE_END