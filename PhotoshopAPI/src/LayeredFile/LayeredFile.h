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

#include <variant>
#include <vector>
#include <memory>


PSAPI_NAMESPACE_BEGIN


// Enumerator to easily switch between traversal directions
enum class LayerOrder
{
	forward,
	reverse
};


template <typename T>
struct LayeredFile
{
	// The root layers in the file, they may contain multiple levels of sub-layers
	std::vector<std::shared_ptr<Layer<T>>> m_Layers;
	
	Enum::BitDepth m_BitDepth = Enum::BitDepth::BD_8;
	Enum::ColorMode m_ColorMode = Enum::ColorMode::RGB;	// Currently we only support RGB but this may change in the future
	uint64_t m_Width = 0u;
	uint64_t m_Height = 0u;

	// Convert the current layeredFile into a PhotoshopFile, transferring all relevant information
	// Note that this will not fill any specific TaggedBlocks or ResourceBlocks beyond what is required
	// to create the layer structure.
	std::unique_ptr<PhotoshopFile> toPhotoshopFile();

	/// Find a layer based on the given path, the path has to be separated by forwards slashes, an example path might look
	/// like this "Group1/GroupNested/ImageLayer". You can retrieve any layers this way and it returns a reference to the specific
	/// layer. If any of the keys are invalid the function will return nullopt and issue a warning, not an error
	std::shared_ptr<Layer<T>> findLayer(std::string path) const;

	// Generate a flat layer stack from either the current root or (if supplied) from the given layer.
	// Use this function if you wish to get the most up to date flat layer stack that is in a correct order
	std::vector<std::shared_ptr<Layer<T>>> generateFlatLayers(std::optional<std::shared_ptr<Layer<T>>> layer, const LayerOrder order) const;

	// Generate a LayeredFile instance from a pointer to a photoshop file, taking ownership of it 
	// and discarding it once we are done with it. This involves transferring from a flat layer hierarchy
	// to a layered file using the lrSectionDivider taggedBlock to identify layer breaks
	LayeredFile(std::unique_ptr<PhotoshopFile> file);

	// Generate an empty LayeredFile object whose layers are yet to be populated
	LayeredFile(Enum::ColorMode colorMode, uint64_t width, uint64_t height) requires std::same_as<T, uint8_t>;
	LayeredFile(Enum::ColorMode colorMode, uint64_t width, uint64_t height) requires std::same_as<T, uint16_t>;
	LayeredFile(Enum::ColorMode colorMode, uint64_t width, uint64_t height) requires std::same_as<T, float32_t>;

	inline std::vector<std::shared_ptr<Layer<T>>>& getFlatLayers() { return m_FlatLayers; };
	inline Enum::Version getVersion() { return m_Version; };
	inline uint16_t getNumChannels() { return m_NumChannels; };
private:
	// We also store pointers to the flat layer hierarchy here for easier traversal.
	// This gets created on initialization and whenever we add a new layer we update
	// it as well. Layer order is not guaranteed and may be random.
	std::vector<std::shared_ptr<Layer<T>>> m_FlatLayers;

	Enum::Version m_Version = Enum::Version::Psd;
	uint16_t m_NumChannels = 0u;

};


namespace LayeredFileImpl
{

	// Build the layer hierarchy from a PhotoshopFile object using the Layer and Mask section with its LayerRecords and ChannelImageData subsections;
	// Returns a vector of nested layer variants which can go to any depth
	template <typename T>
	std::vector<std::shared_ptr<Layer<T>>> buildLayerHierarchy(std::unique_ptr<PhotoshopFile> file);
	// Recursively build a layer hierarchy using the LayerRecords, ChannelImageData and their respective reverse iterators
	// See comments in buildLayerHierarchy on why we iterate in reverse
	template <typename T>
	std::vector<std::shared_ptr<Layer<T>>> buildLayerHierarchyRecurse(
		std::vector<LayerRecord>& layerRecords,
		std::vector<ChannelImageData>& channelImageData,
		std::vector<LayerRecord>::reverse_iterator& layerRecordsIterator,
		std::vector<ChannelImageData>::reverse_iterator& channelImageDataIterator);


	// Identify the type of layer the current layer record represents and return a layerVariant object (std::variant<ImageLayer, GroupLayer ...>)
	// initialized with the given layer record and corresponding channel image data.
	// This function was heavily inspired by the psd-tools library as they have the most coherent parsing of this information
	template <typename T>
	std::shared_ptr<Layer<T>> identifyLayerType(LayerRecord& layerRecord, ChannelImageData& channelImageData);


	// Build a flat layer hierarchy from a nested layer structure and return this vector. Layer order
	// is not guaranteed
	template <typename T>
	std::vector<std::shared_ptr<Layer<T>>> generateFlatLayers(const std::vector<std::shared_ptr<Layer<T>>>& nestedLayers);
	// Recursively build a flat layer hierarchy
	template <typename T>
	void generateFlatLayersRecurse(const std::vector<std::shared_ptr<Layer<T>>>& nestedLayers, std::vector<std::shared_ptr<Layer<T>>>& flatLayers);


	// Find a layer based on a separated path and a parent layer. To be called by LayeredFile::findLayer
	template <typename T>
	std::shared_ptr<Layer<T>> findLayerRecurse(std::shared_ptr<Layer<T>> parentLayer, std::vector<std::string> path, int index);
}

PSAPI_NAMESPACE_END