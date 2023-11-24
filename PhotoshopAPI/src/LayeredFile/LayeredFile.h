#pragma once

#include "Macros.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "Layer.h"
#include "ImageLayer.h"
#include "GroupLayer.h"

#include <span>

PSAPI_NAMESPACE_BEGIN


template <typename T>
struct LayeredFile
{
	
	// The root layers in the file, they may contain multiple levels of sub-layers
	std::vector<layerVariant<T>> m_Layers;

	// Generate a LayeredFile instance from a pointer to a photoshop file, taking ownership of it 
	// and discarding it once we are done with it. This involves transferring from a flat layer hierarchy
	// to a layered file using the lrSectionDivider taggedBlock to identify layer breaks
	LayeredFile(std::unique_ptr<PhotoshopFile> file);
};


namespace LayeredFileImpl
{
	template <typename T>
	std::vector<layerVariant<T>> buildLayerHierarchy(std::unique_ptr<PhotoshopFile> file);

	template <typename T>
	std::vector<layerVariant<T>> buildLayerHierarchyRecurse(const std::vector<LayerRecord>& layerRecords, std::vector<LayerRecord>::reverse_iterator& layerRecordsIterator);
}

PSAPI_NAMESPACE_END