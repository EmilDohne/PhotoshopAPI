
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
LayerAndMaskInformation generate_layermaskinfo(LayeredFile<T>& layeredFile, const FileHeader& header)
{
	PSAPI_LOG_ERROR("LayeredFile", "Cannot construct layer and mask information section if type is not uint8_t, uint16_t or float32_t");
	return LayerAndMaskInformation();
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <>
LayerAndMaskInformation generate_layermaskinfo(LayeredFile<uint8_t>& layeredFile, const FileHeader& header)
{
	LayerInfo lrInfo = generate_layerinfo<uint8_t>(layeredFile, header);
	// This section is mainly there for backwards compatibility it seems and from initial testing
	// does not appear to really be relevant for documents
	GlobalLayerMaskInfo maskInfo{};

	return LayerAndMaskInformation(lrInfo, maskInfo, std::nullopt);
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <>
LayerAndMaskInformation generate_layermaskinfo(LayeredFile<uint16_t>& layeredFile, const FileHeader& header)
{
	LayerInfo emptyLrInfo{};
	LayerInfo lrInfo = generate_layerinfo<uint16_t>(layeredFile, header);
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
LayerAndMaskInformation generate_layermaskinfo(LayeredFile<float32_t>& layeredFile, const FileHeader& header)
{
	LayerInfo emptyLrInfo{};
	LayerInfo lrInfo = generate_layerinfo<float32_t>(layeredFile, header);
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
LayerInfo generate_layerinfo(LayeredFile<T>& layeredFile, const FileHeader& header)
{
	// We must first for each layer generate a layer records as well as channelImageData using the reversed flat layers
	std::vector<std::shared_ptr<Layer<T>>> flatLayers = layeredFile.flat_layers(std::nullopt, LayerOrder::reverse);


	std::vector<LayerRecord> layerRecords;
	std::vector<ChannelImageData> imageData;
	layerRecords.reserve(flatLayers.size());
	imageData.reserve(flatLayers.size());

	for (const auto& layer : flatLayers)
	{
		std::tuple<LayerRecord, ChannelImageData> lrData = generate_layerdata<T>(layeredFile, layer, header);
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
std::tuple<LayerRecord, ChannelImageData> generate_layerdata(LayeredFile<T>& layeredFile, std::shared_ptr<Layer<T>> layer, const FileHeader& header)
{
	// We default to not copying here
	auto lrData = layer->to_photoshop(layeredFile.colormode(), header);
	return lrData;
}


PSAPI_NAMESPACE_END