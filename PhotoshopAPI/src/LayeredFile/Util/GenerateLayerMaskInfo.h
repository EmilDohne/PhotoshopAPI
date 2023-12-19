#pragma once

#include "Macros.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "LayeredFile/LayeredFile.h"

#include <memory>

PSAPI_NAMESPACE_BEGIN


template <typename T>
LayerAndMaskInformation generateLayerMaskInfo(const LayeredFile<T>& layeredFile);


template <typename T>
LayerInfo generateLayerInfo(const LayeredFile<T>& layeredFile);


template <typename T>
LayerRecord generateLayerRecord(const std::shared_ptr<Layer<T>> layer);


template <typename T>
ChannelImageData generateChannelImageData(const std::shared_ptr<Layer<T>> layer);


PSAPI_NAMESPACE_END
