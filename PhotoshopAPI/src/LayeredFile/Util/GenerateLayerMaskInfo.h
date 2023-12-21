#pragma once

#include "Macros.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "LayeredFile/LayeredFile.h"

#include <memory>

PSAPI_NAMESPACE_BEGIN


template <typename T>
LayerAndMaskInformation generateLayerMaskInfo(LayeredFile<T>& layeredFile);


template <typename T>
LayerInfo generateLayerInfo(LayeredFile<T>& layeredFile);


template <typename T>
std::tuple<LayerRecord, ChannelImageData> generateLayerData(LayeredFile<T>& layeredFile, std::shared_ptr<Layer<T>> layer);


PSAPI_NAMESPACE_END
