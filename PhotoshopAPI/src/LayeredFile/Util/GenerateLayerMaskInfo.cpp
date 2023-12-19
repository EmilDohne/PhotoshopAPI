
#include "GenerateLayerMaskInfo.h"

#include "Macros.h"
#include "Struct/TaggedBlock.h"

#include <variant>
#include <optional>
#include <memory>
#include <algorithm>


PSAPI_NAMESPACE_BEGIN


// Since we split the declaration we must explicitly instantiate these template functions.
// Fortunately the types are well known ahead of time
template LayerInfo generateLayerInfo(const LayeredFile<uint8_t>& layeredFile);
template LayerInfo generateLayerInfo(const LayeredFile<uint16_t>& layeredFile);
template LayerInfo generateLayerInfo(const LayeredFile<float32_t>& layeredFile);

template LayerRecord generateLayerRecord(const std::shared_ptr<Layer<uint8_t>> layer);
template LayerRecord generateLayerRecord(const std::shared_ptr<Layer<uint16_t>> layer);
template LayerRecord generateLayerRecord(const std::shared_ptr<Layer<float32_t>> layer);

template ChannelImageData generateChannelImageData(const std::shared_ptr<Layer<uint8_t>> layer);
template ChannelImageData generateChannelImageData(const std::shared_ptr<Layer<uint16_t>> layer);
template ChannelImageData generateChannelImageData(const std::shared_ptr<Layer<float32_t>> layer);


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
LayerAndMaskInformation generateLayerMaskInfo(const LayeredFile<T>& layeredFile)
{
	PSAPI_LOG_ERROR("LayeredFile", "Cannot construct layer and mask information section if type is not uint8_t, uint16_t or float32_t")
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <>
LayerAndMaskInformation generateLayerMaskInfo(const LayeredFile<uint8_t>& layeredFile)
{
	LayerInfo lrInfo = generateLayerInfo<uint8_t>(layeredFile);
	// This section is mainly there for backwards compatibility it seems and from initial testing
	// does not appear to really be relevant for documents
	GlobalLayerMaskInfo maskInfo{};

	return LayerAndMaskInformation(lrInfo, maskInfo, std::nullopt);
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <>
LayerAndMaskInformation generateLayerMaskInfo(const LayeredFile<uint16_t>& layeredFile)
{
	LayerInfo emptyLrInfo{};
	LayerInfo lrInfo = generateLayerInfo<uint16_t>(layeredFile);
	// This section is mainly there for backwards compatibility it seems and from initial testing
	// does not appear to really be relevant for documents
	GlobalLayerMaskInfo maskInfo{};

	std::vector<std::shared_ptr<TaggedBlock>> blockPtrs{};
	blockPtrs.push_back(std::make_shared<Lr16TaggedBlock>(lrInfo));
	TaggedBlockStorage blockStorage(blockPtrs);

	return LayerAndMaskInformation(emptyLrInfo, maskInfo, std::make_optional<AdditionalLayerInfo>(blockStorage));
	
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <>
LayerAndMaskInformation generateLayerMaskInfo(const LayeredFile<float32_t>& layeredFile)
{
	LayerInfo emptyLrInfo{};
	LayerInfo lrInfo = generateLayerInfo<float32_t>(layeredFile);
	// This section is mainly there for backwards compatibility it seems and from initial testing
	// does not appear to really be relevant for documents
	GlobalLayerMaskInfo maskInfo{};

	std::vector<std::shared_ptr<TaggedBlock>> blockPtrs{};
	blockPtrs.push_back(std::make_shared<Lr32TaggedBlock>(lrInfo));
	TaggedBlockStorage blockStorage(blockPtrs);

	return LayerAndMaskInformation(emptyLrInfo, maskInfo, std::make_optional<AdditionalLayerInfo>(blockStorage));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
LayerInfo generateLayerInfo(const LayeredFile<T>& layeredFile)
{
	// We must first for each layer generate a layer records as well as channelImageData using the reversed flat layers
	std::vector<std::shared_ptr<Layer<T>>> flatLayers = layeredFile.generateFlatLayers(std::nullopt, LayerOrder::reverse);


	std::vector<LayerRecord> layerRecords;
	std::vector<ChannelImageData> imageData;
	layerRecords.reserve(flatLayers.size());
	imageData.reserve(flatLayers.size());

	for (const auto& layer : flatLayers)
	{
		LayerRecord lrRecord = generateLayerRecord<T>(layer);
		ChannelImageData lrImageData = generateChannelImageData<T>(layer);
	}

	return LayerInfo(layerRecords, imageData);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
LayerRecord generateLayerRecord(const std::shared_ptr<Layer<T>> layer)
{

}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
ChannelImageData generateChannelImageData(const std::shared_ptr<Layer<T>> layer)
{

}



PSAPI_NAMESPACE_END