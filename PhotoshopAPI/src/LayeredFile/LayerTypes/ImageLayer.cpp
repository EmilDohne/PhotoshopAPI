#include "ImageLayer.h"

#include "Layer.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"

#include <vector>
#include <unordered_map>
#include <optional>

PSAPI_NAMESPACE_BEGIN


// Instantiate the template types for ImageLayer
template struct ImageLayer<uint8_t>;
template struct ImageLayer<uint16_t>;
template struct ImageLayer<float32_t>;



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::tuple<LayerRecord, ChannelImageData> ImageLayer<T>::toPhotoshop(const Enum::ColorMode colorMode, const bool doCopy)
{
	PascalString lrName = Layer<T>::generatePascalString();
	auto extents = Layer<T>::generateExtents();
	int32_t top = std::get<0>(extents);
	int32_t left = std::get<1>(extents);
	int32_t bottom = std::get<2>(extents);
	int32_t right = std::get<3>(extents);
	uint16_t channelCount = m_ImageData.size() + static_cast<uint16_t>(m_LayerMask.has_value());

	// Initialize the channel information as well as the channel image data, the size held in the channelInfo might change depending on
	// the compression mode chosen on export and must therefore be updated later.
	auto channelData = this->extractImageData(doCopy);
	auto& channelInfoVec = std::get<0>(channelData);
	auto channelImgData = std::move(std::get<1>(channelData));

	uint8_t clipping = 0u;	// No clipping mask for now
	LayerRecords::BitFlags bitFlags(false, m_IsVisible, false);
	std::optional<LayerRecords::LayerMaskData> lrMaskData = Layer<T>::generateMaskData();
	LayerRecords::LayerBlendingRanges blendingRanges = Layer<T>::generateBlendingRanges(colorMode);
	
	LayerRecord lrRecord = LayerRecord(
		lrName,
		top,
		left,
		bottom,
		right,
		channelCount,
		channelInfoVec,
		m_BlendMode,
		m_Opacity,
		clipping,
		bitFlags,
		lrMaskData,
		blendingRanges,
		std::nullopt	// We dont really need to pass any additional layer info in here
	);

	return std::make_tuple(lrRecord, channelImgData);
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


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::tuple<std::vector<LayerRecords::ChannelInformation>, ChannelImageData> ImageLayer<T>::extractImageData(const bool doCopy)
{
	std::vector<LayerRecords::ChannelInformation> channelInfoVec;
	std::vector<std::unique_ptr<BaseImageChannel> channelDataVec;

	// First extract our mask data, the order of our channels does not matter as long as the 
	// order of channelInfo and channelData is the same
	auto maskData = Layer<T>::extractLayerMask(doCopy);
	if (maskData.has_value())
	{
		channelInfoVec.push_back(std::get<0>(maskData.value()));
		channelDataVec.push_back(std::move(std::get<1>(maskData.value())));
	}

	// Extract all the channels next and push them into our data representation
	for (const auto& it : m_ImageData)
	{
		channelInfoVec.push_back(LayerRecords::ChannelInformation{ it.first, it.second.m_OrigSize });
		if (doCopy)
			ChannelDataVec.push_back(std::make_unique<BaseImageChannel>(it.second));
		else
			ChannelDataVec.push_back(std::make_unique<BaseImageChannel>(std::move(it.second)));
	}

	// Construct the channel image data from our vector of ptrs, moving gets handled by the constructor
	ChannelImageData channelData(channelDataVec);

	return std::make_tuple(channelInfoVec, channelData);
}


PSAPI_NAMESPACE_END