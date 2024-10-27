#pragma once

#include "Macros.h"
#include "Layer.h"

#include "Core/Struct/DescriptorStructure.h"
#include "Core/Warp/SmartObjectWarp.h"

#include <fstream>
#include "fmt/core.h"


PSAPI_NAMESPACE_BEGIN

namespace SmartObject
{
	struct Transform
	{

	};

	struct Warp
	{

	};
}

/// This struct holds no data, we just use it to identify its type.
/// We could hold references here 
template <typename T>
struct SmartObjectLayer : Layer<T>
{
	SmartObjectLayer() = default;

	SmartObjectLayer(const LayerRecord& layerRecord, ChannelImageData& channelImageData, const FileHeader& header, const AdditionalLayerInfo& globalAdditionalLayerInfo) 
		: Layer<T>(layerRecord, channelImageData, header)
	{
		// Local and global additional layer info in this case refer to the one stored on the individual layer and the one 
		// stored on the LayerAndMaskInfo section respectively
		if (layerRecord.m_AdditionalLayerInfo)
		{
			const auto& localAdditionalLayerInfo = layerRecord.m_AdditionalLayerInfo.value();
			decode(localAdditionalLayerInfo, globalAdditionalLayerInfo, Layer<T>::m_LayerName);
		}
		else
		{
			PSAPI_LOG_ERROR("SmartObject", "Internal Error: Expected smart object layer to contain an AdditionalLayerInfo section");
		}
	}

	std::vector<T> getImageData()
	{
		return {};
	}


private:

	void decode(const AdditionalLayerInfo& local, const AdditionalLayerInfo& global, const std::string& name)
	{
		// Get the LinkedLayers from the global additional layer info
		auto g_linkedLayer = global.getTaggedBlock<LinkedLayerTaggedBlock>(Enum::TaggedBlockKey::lrLinked);
		if (!g_linkedLayer)
		{
			g_linkedLayer = global.getTaggedBlock<LinkedLayerTaggedBlock>(Enum::TaggedBlockKey::lrLinked_8Byte);
		}
		const auto l_placedLayer = local.getTaggedBlock<PlacedLayerTaggedBlock>(Enum::TaggedBlockKey::lrPlaced);
		const auto l_placedLayerData = local.getTaggedBlock<PlacedLayerDataTaggedBlock>(Enum::TaggedBlockKey::lrPlacedData);

		// Prefer decoding via placed layer data as that is more up to date
		if (g_linkedLayer && l_placedLayerData)
		{
			decodePlacedLayerData(l_placedLayerData.value(), g_linkedLayer.value(), name);
		}
		// Fallback to placed layer if placed layer data wasn't present
		else if (g_linkedLayer && l_placedLayer)
		{
			decodePlacedLayer(l_placedLayer.value(), g_linkedLayer.value(), name); // Call decodePlacedLayer here instead of decodePlacedLayerData
		}
		else
		{
			PSAPI_LOG_ERROR("SmartObject", "Internal Error: Unable to decode SmartObject layer '%s' as we couldn't find the appropriate tagged blocks", name.c_str());
		}
	}

	void decodePlacedLayerData(const std::shared_ptr<PlacedLayerDataTaggedBlock>& local, const std::shared_ptr<LinkedLayerTaggedBlock>& global, const std::string& name)
	{
		const auto& descriptor = local->m_Descriptor;

		/// TMP
		auto json = descriptor.to_json();
		auto filename = fmt::format("C:/Users/emild/Desktop/linkedlayers/warp/{}.json", name);
		std::ofstream file(filename);
		file << json;
		std::cout << "Wrote file: " << filename << std::endl;


		const auto& identifier = descriptor.at("Idnt");	// The identifier that maps back to the LinkedLayer

		// These we all ignore for the time being, we store them locally and just rewrite them back out later
		const auto& _placed = descriptor.at("placed");
		const auto& _page_num = descriptor.at("PgNm");
		const auto& _total_pages = descriptor.at("totalPages");
		const auto& _crop = descriptor.at("Crop");
		const auto& _frame_step = descriptor.at("frameStep");
		const auto& _duration = descriptor.at("duration");
		const auto& _frame_count = descriptor.at("frameCount");
		const auto& _anti_alias = descriptor.at("Annt");

		const auto& type = descriptor.at("Type");
		const auto& transform = descriptor.at("Trnf");
		const auto& non_affine_transform = descriptor.at("nonAffineTransform");
		const auto& warp = descriptor.at<Descriptors::Descriptor>("warp");
		auto warpStruct = SmartObjectWarp::deserialize(warp);
		const auto& size = descriptor.at("Sz  ");		// The spaces are not a mistake
		const auto& resolution = descriptor.at("Rslt");	// In DPI

		const auto& _comp = descriptor.at("comp");
		const auto& _comp_info = descriptor.at("compInfo");

		PSAPI_LOG("A", "A");
	}


	void decodePlacedLayer(const std::shared_ptr<PlacedLayerTaggedBlock>& local, const std::shared_ptr<LinkedLayerTaggedBlock>& global, const std::string& name)
	{

	}

	std::string m_UUID;
	std::string m_Filename;

	std::vector<uint8_t> m_RawBytes;
};


PSAPI_NAMESPACE_END