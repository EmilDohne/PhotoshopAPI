#include "GroupLayer.h"

PSAPI_NAMESPACE_BEGIN


// Instantiate the template types for GroupLayer
template struct GroupLayer<uint8_t>;
template struct GroupLayer<uint16_t>;
template struct GroupLayer<float32_t>;


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
	uint8_t clipping = 0u;	// No clipping mask for now
	uint8_t bitFlags = 1u << 1;	// Set the layer to be visible
	std::optional<LayerRecords::LayerMaskData> lrMaskData = Layer<T>::generateMaskData();
	LayerRecords::LayerBlendingRanges blendingRanges = Layer<T>::generateBlendingRanges(colorMode);


	// Initialize the channelInfo. Note that if the data is to be compressed the channel size gets update
	// again later
	std::vector<LayerRecords::ChannelInformation> channelInfo;
	std::vector<ChannelImageData> imageChannels;
	if (m_LayerMask.has_value())
	{
		auto& maskImgChannel = m_LayerMask.value().maskData;
		Enum::ChannelIDInfo maskIdInfo{ Enum::ChannelID::UserSuppliedLayerMask, -2 };
		channelInfo.push_back(LayerRecord::m_ChannelInformation{ maskIdInfo, maskImgChannel.m_OrigSize });
		ChannelImageData channelImgData{};
		if (doCopy)
			channelImgData.m_ImageData = maskImgChannel;
		else
			channelImgData.m_ImageData = std::move(maskImgChannel);
		imageChannels.push_back(ChannelImageData(std::make_unique<BaseImageChannel>(channelImgData));
	}

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
GroupLayer<T>::GroupLayer(const LayerRecord& layerRecord, const ChannelImageData& channelImageData) : Layer<T>(layerRecord, channelImageData)
{
	// Because Photoshop stores the Passthrough blend mode on the layer section divider tagged block we must check if it present here
	if (!layerRecord.m_AdditionalLayerInfo.has_value()) return;
	const auto& taggedBlocks = layerRecord.m_AdditionalLayerInfo.value().m_TaggedBlocks;
	const auto lrSectionBlockPtr = taggedBlocks.getTaggedBlockView<LrSectionTaggedBlock>(Enum::TaggedBlockKey::lrSectionDivider);
	if (!lrSectionBlockPtr) return;

	if (lrSectionBlockPtr.m_BlendMode.has_value())
	{
		m_BlendMode = lrSectionBlockPtr.m_BlendMode.value();
	}
	if (lrSectionBlockPtr.m_Type == Enum::SectionDivider::ClosedFolder)
	{
		m_isCollapsed = true;
	}
}


PSAPI_NAMESPACE_END