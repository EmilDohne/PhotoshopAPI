#pragma once

#include "Macros.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "LayeredFile/LayeredFile.h"

#include <memory>

PSAPI_NAMESPACE_BEGIN


// Generate a layer and mask information section based on the information in the LayeredFile
template <typename T>
LayerAndMaskInformation generate_layermaskinfo(LayeredFile<T>& layeredFile, const FileHeader& header, std::filesystem::path file_path);


template <typename T>
LayerInfo generate_layerinfo(LayeredFile<T>& layeredFile, const FileHeader& header);


// Generates the accompanying layer data (LayerRecord and ChannelImageData) for each of the layers in the scene
template <typename T>
std::tuple<LayerRecord, ChannelImageData> generate_layerdata(LayeredFile<T>& layeredFile, std::shared_ptr<Layer<T>> layer, const FileHeader& header);


PSAPI_NAMESPACE_END
