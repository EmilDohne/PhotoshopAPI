#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Layer.h"
#include "Struct/ImageChannel.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"

#include <unordered_map>

PSAPI_NAMESPACE_BEGIN

// A pixel based image layer
template <typename T>
struct ImageLayer : public Layer<T>
{
	// Store the image data as a per-channel map to be used later
	std::unordered_map<Enum::ChannelIDInfo, ImageChannel<T>> m_ImageData;

	// Generate a photoshop layerRecords and imageData based on the current layer
	std::tuple<LayerRecord, ChannelImageData> toPhotoshop(const Enum::ColorMode colorMode);

	ImageLayer(const LayerRecord& layerRecord, const ChannelImageData& channelImageData);
};

PSAPI_NAMESPACE_END