#include "ImageLayer.h"

#include "Layer.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"

#include <vector>
#include <unordered_map>

PSAPI_NAMESPACE_BEGIN


// Instantiate the template types for ImageLayer
template struct ImageLayer<uint8_t>;
template struct ImageLayer<uint16_t>;
template struct ImageLayer<float32_t>;



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::tuple<LayerRecord, std::vector<ChannelImageData>> ImageLayer<T>::toPhotoshop(const Enum::ColorMode colorMode)
{
	PascalString lrName = Layer<T>::generatePascalString();
	auto extents = Layer<T>::generateExtents();
	int32_t top = std::get<0>(extents);
	int32_t left = std::get<1>(extents);
	int32_t bottom = std::get<2>(extents);
	int32_t right = std::get<3>(extents);
	uint16_t channelCount = m_ImageData.size() + static_cast<uint16_t>(m_LayerMask.has_value());
	// Initialize the channelInfo. Note that if the data is to be compressed the channel size gets update
	// again later
	std::vector<LayerRecords::ChannelInformation> channelInfo;
	std::vector<ChannelImageData> imageChannels;
	for (const auto& it : m_ImageData)
	{
		channelInfo.push_back(LayerRecords::ChannelInformation{ it.first.id, it.second.m_OrigSize });
		ChannelImageData channelImgData{};
		channelImgData.m_ImageData = std::move(it.second);
		imageChannels.push_back(ChannelImageData(std::make_unique<BaseImageChannel>(channelImgData));
	}
	if (m_LayerMask.has_value())
	{
		auto& maskImgChannel = m_LayerMask.value().maskData;
		Enum::ChannelIDInfo maskIdInfo{ Enum::ChannelID::UserSuppliedLayerMask, -2 };
		channelInfo.push_back(LayerRecord::m_ChannelInformation{ maskIdInfo, maskImgChannel.m_OrigSize });
		ChannelImageData channelImgData{};
		channelImgData.m_ImageData = std::move(maskImgChannel);
		imageChannels.push_back(ChannelImageData(std::make_unique<BaseImageChannel>(channelImgData));
	}

	uint8_t clipping = 0u;	// No clipping mask for now
	uint8_t bitFlags = 1u << 1;	// Set the layer to be visible
	std::optional<LayerRecords::LayerMaskData> lrMaskData = Layer<T>::generateMaskData();
	LayerRecords::LayerBlendingRanges blendingRanges = Layer<T>::generateBlendingRanges(colorMode);
	
	LayerRecord lrRecord = LayerRecord(
		lrName,
		top,
		left,
		bottom,
		right,
		channelCount,
		channelInfo,
		m_BlendMode,
		m_Opacity,
		clipping,
		bitFlags,
		lrMaskData,
		blendingRanges,
		std::nullopt	// We dont really need to pass any additional layer info in here
	);
	return std::make_tuple(lrRecord, imageChannels);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
ImageLayer<T>::ImageLayer(const LayerRecord& layerRecord, const ChannelImageData& channelImageData) : 
	Layer<T>(layerRecord, channelImageData)
{
	// Move the layers into our own layer representation
	for (int i = 0; i < layerRecord.m_ChannelCount; ++i)
	{
		auto& channelInfo = layerRecord.m_ChannelInformation[i];
		auto channelPtr = channelImageData.extractImagePtr(channelInfo.m_ChannelID);
		// Pointers might have already been invalidated such as extracting masks beforehand
		if (!channelPtr) continue;

		// Insert any valid pointers to channels we have
		if (auto imageChannelPtr = dynamic_cast<ImageChannel<T>*>(channelPtr.get())
		{
			m_ImageData[channelInfo.m_ChannelID] = std::move(*channelPtr);
		}
	}
}




PSAPI_NAMESPACE_END