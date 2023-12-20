#pragma once

#include "Macros.h"
#include "Layer.h"
#include "ImageLayer.h"
#include "SectionDividerLayer.h"
#include "ArtboardLayer.h"
#include "ShapeLayer.h"
#include "SmartObjectLayer.h"
#include "TextLayer.h"
#include "AdjustmentLayer.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"

#include <vector>
#include <variant>
#include <memory>


PSAPI_NAMESPACE_BEGIN

// A layer representing a group of 0 or more other layers. This may be nested 
template <typename T>
struct GroupLayer : public Layer<T>
{
	std::vector<std::shared_ptr<Layer<T>>> m_Layers;
	bool m_isCollapsed = false;		// Specifies whether or not the layer is collapsed or open

	// Generate a photoshop layerRecords and imageData based on the current layer
	std::tuple<LayerRecord, std::vector<ChannelImageData>> toPhotoshop(const Enum::ColorMode colorMode);

	GroupLayer(const LayerRecord& layerRecord, const ChannelImageData& channelImageData);
};


PSAPI_NAMESPACE_END