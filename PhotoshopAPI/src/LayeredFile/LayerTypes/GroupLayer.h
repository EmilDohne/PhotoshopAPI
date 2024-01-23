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

	// Insert a layer under the GroupLayer
	void addLayer(std::shared_ptr<Layer<T>> layer);

	// Generate a photoshop layerRecords and imageData based on the current layer. if doCopy is set to false this will likely
	// invalidate our m_LayerMask since we perform move operations on them. If doCopy is 
	// set to true we can safely keep using the GroupLayer instance. it is advised to only set doCopy to false on parsing of
	// the whole layeredFile -> PhotoshopFile.
	std::tuple<LayerRecord, ChannelImageData> toPhotoshop(const Enum::ColorMode colorMode, const bool doCopy, const FileHeader& header) override;

	GroupLayer(const LayerRecord& layerRecord, ChannelImageData& channelImageData);

	// Generate a Group Layer instance with the given layer parameters. isCollapsed specifies whether the group is to be shown as open or closed
	GroupLayer(const Layer<T>::Params& layerParameters, bool isCollapsed = false);

private:
	// Generate an additional layer information section which holds information about the group state
	AdditionalLayerInfo generateAdditionalLayerInfo();
};


PSAPI_NAMESPACE_END