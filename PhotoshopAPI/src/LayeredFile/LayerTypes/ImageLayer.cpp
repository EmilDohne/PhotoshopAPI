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
std::tuple<LayerRecord, ChannelImageData> ImageLayer<T>::toPhotoshop(const Enum::ColorMode colorMode)
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
	for (const auto& it : m_ImageData)
	{
		channelInfo.push_back(LayerRecords::ChannelInformation{ it.first.id, it.second.m_OrigSize });
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
	return lrRecord;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
ImageLayer<T>::ImageLayer(const LayerRecord& layerRecord, const ChannelImageData& channelImageData) : 
	Layer<T>(layerRecord, channelImageData)
{
	
}




PSAPI_NAMESPACE_END