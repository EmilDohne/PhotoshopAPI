#pragma once

#include "Macros.h"
#include "Layer.h"
#include "ImageLayer.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"

#include <vector>
#include <memory>


PSAPI_NAMESPACE_BEGIN

template <typename T>
struct GroupLayer;

template <typename T>
using layerVariant = std::variant<ImageLayer<T>, GroupLayer<T>>;

template <typename T>
struct GroupLayer : public Layer<T>
{
	std::vector<layerVariant<T>> m_Layers;

	GroupLayer(const LayerRecord& layerRecord);
};




PSAPI_NAMESPACE_END