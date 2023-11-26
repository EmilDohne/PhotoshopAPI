#pragma once

#include "Macros.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"

#include "LayerTypes/Layer.h"
#include "LayerTypes/ImageLayer.h"
#include "LayerTypes/GroupLayer.h"
#include "LayerTypes/AdjustmentLayer.h"
#include "LayerTypes/ArtboardLayer.h"
#include "LayerTypes/SectionDividerLayer.h"
#include "LayerTypes/ShapeLayer.h"
#include "LayerTypes/SmartObjectLayer.h"
#include "LayerTypes/TextLayer.h"


PSAPI_NAMESPACE_BEGIN


template <typename T>
struct LayeredFile
{
	
	// The root layers in the file, they may contain multiple levels of sub-layers
	std::vector<layerVariant<T>> m_Layers;

	Enum::BitDepth m_BitDepth = Enum::BitDepth::BD_8;
	Enum::ColorMode m_ColorMode = Enum::ColorMode::RGB;	// Currently we only support RGB but this may change in the future
	uint64_t m_Width = 0u;
	uint64_t m_Height = 0u;

	// Generate a LayeredFile instance from a pointer to a photoshop file, taking ownership of it 
	// and discarding it once we are done with it. This involves transferring from a flat layer hierarchy
	// to a layered file using the lrSectionDivider taggedBlock to identify layer breaks
	LayeredFile(std::unique_ptr<PhotoshopFile> file);

	// Generate an empty LayeredFile object whose layers are yet to be populated
	LayeredFile(Enum::ColorMode colorMode, uint64_t width, uint64_t height) requires std::same_as<T, uint8_t>;
	LayeredFile(Enum::ColorMode colorMode, uint64_t width, uint64_t height) requires std::same_as<T, uint16_t>;
	LayeredFile(Enum::ColorMode colorMode, uint64_t width, uint64_t height) requires std::same_as<T, float32_t>;

	LayeredFile(Enum::BitDepth bitDepth, Enum::ColorMode colorMode, uint64_t width, uint64_t height);
};


namespace LayeredFileImpl
{

	// Build the layer hierarchy from a PhotoshopFile object using the Layer and Mask section with its LayerRecords and ChannelImageData subsections;
	// Returns a vector of nested layer variants which can go to any depth
	template <typename T>
	std::vector<layerVariant<T>> buildLayerHierarchy(std::unique_ptr<PhotoshopFile> file);


	// Recursively build a layer hierarchy using the LayerRecords, ChannelImageData and their respective reverse iterators
	// See comments in buildLayerHierarchy on why we iterate in reverse
	template <typename T>
	std::vector<layerVariant<T>> buildLayerHierarchyRecurse(
		const std::vector<LayerRecord>& layerRecords,
		const std::vector<std::shared_ptr<ChannelImageData<T>>>& channelImageData,
		std::vector<LayerRecord>::reverse_iterator& layerRecordsIterator,
		typename std::vector<std::shared_ptr<ChannelImageData<T>>>::reverse_iterator& channelImageDataIterator);


	// Identify the type of layer the current layer record represents and return a layerVariant object (std::variant<ImageLayer, GroupLayer ...>)
	// initialized with the given layer record and corresponding channel image data.
	// This function was heavily inspired by the psd-tools library as they have the most coherent parsing of this information
	template <typename T>
	layerVariant<T> identifyLayerType(const LayerRecord& layerRecord, std::shared_ptr<ChannelImageData<T>> channelImageData);

}

PSAPI_NAMESPACE_END