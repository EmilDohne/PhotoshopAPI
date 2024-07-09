
#include "GenerateLayerMaskInfo.h"

#include "Macros.h"
#include "Core/Struct/TaggedBlock.h"
#include "LayeredFile/LayerTypes/Layer.h"

#include <variant>
#include <optional>
#include <memory>
#include <algorithm>


PSAPI_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
LayerAndMaskInformation generateLayerMaskInfo(LayeredFile<T>& layeredFile, const FileHeader& header)
{
	PSAPI_LOG_ERROR("LayeredFile", "Cannot construct layer and mask information section if type is not uint8_t, uint16_t or float32_t");
	return LayerAndMaskInformation();
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <>
LayerAndMaskInformation generateLayerMaskInfo(LayeredFile<uint8_t>& layeredFile, const FileHeader& header)
{
	LayerInfo lrInfo = generateLayerInfo<uint8_t>(layeredFile, header);
	// This section is mainly there for backwards compatibility it seems and from initial testing
	// does not appear to really be relevant for documents
	GlobalLayerMaskInfo maskInfo{};

	return LayerAndMaskInformation(lrInfo, maskInfo, std::nullopt);
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <>
LayerAndMaskInformation generateLayerMaskInfo(LayeredFile<uint16_t>& layeredFile, const FileHeader& header)
{
	LayerInfo emptyLrInfo{};
	LayerInfo lrInfo = generateLayerInfo<uint16_t>(layeredFile, header);
	// This section is mainly there for backwards compatibility it seems and from initial testing
	// does not appear to really be relevant for documents
	GlobalLayerMaskInfo maskInfo{};

	std::vector<std::shared_ptr<TaggedBlock>> blockPtrs{};
	blockPtrs.push_back(std::make_shared<Lr16TaggedBlock>(lrInfo, header));
	TaggedBlockStorage blockStorage(blockPtrs);

	return LayerAndMaskInformation(emptyLrInfo, maskInfo, std::make_optional<AdditionalLayerInfo>(blockStorage));
	
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <>
LayerAndMaskInformation generateLayerMaskInfo(LayeredFile<float32_t>& layeredFile, const FileHeader& header)
{
	LayerInfo emptyLrInfo{};
	LayerInfo lrInfo = generateLayerInfo<float32_t>(layeredFile, header);
	// This section is mainly there for backwards compatibility it seems and from initial testing
	// does not appear to really be relevant for documents
	GlobalLayerMaskInfo maskInfo{};

	std::vector<std::shared_ptr<TaggedBlock>> blockPtrs{};
	blockPtrs.push_back(std::make_shared<Lr32TaggedBlock>(lrInfo, header));
	TaggedBlockStorage blockStorage(blockPtrs);

	return LayerAndMaskInformation(emptyLrInfo, maskInfo, std::make_optional<AdditionalLayerInfo>(blockStorage));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
LayerInfo generateLayerInfo(LayeredFile<T>& layeredFile, const FileHeader& header)
{
	// We must first for each layer generate a layer records as well as channelImageData using the reversed flat layers
	std::vector<std::shared_ptr<Layer<T>>> flatLayers = layeredFile.generateFlatLayers(std::nullopt, LayerOrder::reverse);


	std::vector<LayerRecord> layerRecords;
	std::vector<ChannelImageData> imageData;
	layerRecords.reserve(flatLayers.size());
	imageData.reserve(flatLayers.size());

	for (const auto& layer : flatLayers)
	{
		std::tuple<LayerRecord, ChannelImageData> lrData = generateLayerData<T>(layeredFile, layer, header);
		LayerRecord lrRecord = std::move(std::get<0>(lrData));
		ChannelImageData lrImageData = std::move(std::get<1>(lrData));

		layerRecords.push_back(std::move(lrRecord));
		imageData.push_back(std::move(lrImageData));
	}

	return LayerInfo(std::move(layerRecords), std::move(imageData));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::tuple<LayerRecord, ChannelImageData> generateLayerData(LayeredFile<T>& layeredFile, std::shared_ptr<Layer<T>> layer, const FileHeader& header)
{
	// We default to not copying here
	auto lrData = layer->toPhotoshop(layeredFile.m_ColorMode, header);
	return lrData;
}


PSAPI_NAMESPACE_END