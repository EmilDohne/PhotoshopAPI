#include "GroupLayer.h"

#include "Macros.h"
#include "Struct/TaggedBlock.h"
#include "Struct/TaggedBlockStorage.h"


PSAPI_NAMESPACE_BEGIN


// Instantiate the template types for GroupLayer
template struct GroupLayer<uint8_t>;
template struct GroupLayer<uint16_t>;
template struct GroupLayer<float32_t>;


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::tuple<LayerRecord, ChannelImageData> GroupLayer<T>::toPhotoshop(const Enum::ColorMode colorMode, const bool doCopy)
{
	PascalString lrName = Layer<T>::generatePascalString();
	auto extents = Layer<T>::generateExtents();
	int32_t top = std::get<0>(extents);
	int32_t left = std::get<1>(extents);
	int32_t bottom = std::get<2>(extents);
	int32_t right = std::get<3>(extents);
	uint16_t channelCount = static_cast<uint16_t>(Layer<T>::m_LayerMask.has_value());
	uint8_t clipping = 0u;	// No clipping mask for now
	LayerRecords::BitFlags bitFlags = LayerRecords::BitFlags(false, Layer<T>::m_IsVisible, false);
	std::optional<LayerRecords::LayerMaskData> lrMaskData = Layer<T>::generateMaskData();
	LayerRecords::LayerBlendingRanges blendingRanges = Layer<T>::generateBlendingRanges(colorMode);


	// Initialize the channelInfo. Note that if the data is to be compressed the channel size gets update
	// again later
	std::vector<LayerRecords::ChannelInformation> channelInfoVec;
	std::vector<std::unique_ptr<BaseImageChannel>> channelDataVec;

	// First extract our mask data, the order of our channels does not matter as long as the 
	// order of channelInfo and channelData is the same
	auto maskData = Layer<T>::extractLayerMask(doCopy);
	if (maskData.has_value())
	{
		channelInfoVec.push_back(std::get<0>(maskData.value()));
		channelDataVec.push_back(std::move(std::get<1>(maskData.value())));
	}


	LayerRecord lrRecord = LayerRecord(
		lrName,
		top,
		left,
		bottom,
		right,
		channelCount,
		channelInfoVec,
		Layer<T>::m_BlendMode,
		Layer<T>::m_Opacity,
		clipping,
		bitFlags,
		lrMaskData,
		blendingRanges,
		this->generateAdditionalLayerInfo()
	);
	return std::make_tuple(std::move(lrRecord), ChannelImageData(std::move(channelDataVec)));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
GroupLayer<T>::GroupLayer(const LayerRecord& layerRecord, ChannelImageData& channelImageData) : Layer<T>(layerRecord, channelImageData)
{
	// Because Photoshop stores the Passthrough blend mode on the layer section divider tagged block we must check if it present here
	if (!layerRecord.m_AdditionalLayerInfo.has_value()) return;
	const auto& taggedBlocks = layerRecord.m_AdditionalLayerInfo.value().m_TaggedBlocks;
	const auto lrSectionBlockPtr = taggedBlocks.getTaggedBlockView<LrSectionTaggedBlock>(Enum::TaggedBlockKey::lrSectionDivider);
	if (!lrSectionBlockPtr) return;

	if (lrSectionBlockPtr->m_BlendMode.has_value())
	{
		Layer<T>::m_BlendMode = lrSectionBlockPtr->m_BlendMode.value();
	}
	if (lrSectionBlockPtr->m_Type == Enum::SectionDivider::ClosedFolder)
	{
		m_isCollapsed = true;
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
AdditionalLayerInfo GroupLayer<T>::generateAdditionalLayerInfo()
{
	LrSectionTaggedBlock sectionBlock;
	if (m_isCollapsed)
	{
		if (Layer<T>::m_BlendMode == Enum::BlendMode::Passthrough)
		{
			sectionBlock = LrSectionTaggedBlock(Enum::SectionDivider::ClosedFolder, std::make_optional(Enum::BlendMode::Passthrough));
		}
		else
		{
			sectionBlock = LrSectionTaggedBlock(Enum::SectionDivider::ClosedFolder, std::nullopt);
		}
	}
	else
	{
		if (Layer<T>::m_BlendMode == Enum::BlendMode::Passthrough)
		{
			sectionBlock = LrSectionTaggedBlock(Enum::SectionDivider::OpenFolder, std::make_optional(Enum::BlendMode::Passthrough));
		}
		else
		{
			sectionBlock = LrSectionTaggedBlock(Enum::SectionDivider::OpenFolder, std::nullopt);
		}
	}
	std::vector<std::shared_ptr<TaggedBlock>> blockVec;
	blockVec.push_back(std::make_shared<TaggedBlock>(sectionBlock));
	TaggedBlockStorage blockStorage(blockVec);
	AdditionalLayerInfo lrInfo(blockStorage);
	return lrInfo;
}


PSAPI_NAMESPACE_END