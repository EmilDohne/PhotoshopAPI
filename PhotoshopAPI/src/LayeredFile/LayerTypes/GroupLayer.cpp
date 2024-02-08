#include "GroupLayer.h"

#include "Macros.h"
#include "Struct/TaggedBlock.h"
#include "Struct/TaggedBlockStorage.h"
#include "LayeredFile/LayeredFile.h"


PSAPI_NAMESPACE_BEGIN


// Instantiate the template types for GroupLayer
template struct GroupLayer<uint8_t>;
template struct GroupLayer<uint16_t>;
template struct GroupLayer<float32_t>;


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
void GroupLayer<T>::addLayer(const LayeredFile<T>& layeredFile, std::shared_ptr<Layer<T>> layer)
{
	if (layeredFile.isLayerInDocument(layer))
	{
		PSAPI_LOG_WARNING("GroupLayer", "Cannot insert a layer into the document twice, please use a unique layer. Skipping layer '%s'", layer->m_LayerName.c_str());
		return;
	}
	m_Layers.push_back(layer);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
void GroupLayer<T>::removeLayer(const int index)
{
	if (index >= m_Layers.size())
	{
		PSAPI_LOG_WARNING("GroupLayer", "Cannot remove index %i from the group as it would exceed the amount of layers in the group", index);
		return;
	}
	m_Layers.erase(m_Layers.begin() + index);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
void GroupLayer<T>::removeLayer(std::shared_ptr<Layer<T>>& layer)
{
	int index = 0;
	for (auto& sceneLayer : m_Layers)
	{
		if (layer == sceneLayer)
		{
			m_Layers.erase(m_Layers.begin() + index);
			return;
		}
		++index;
	}
	PSAPI_LOG_WARNING("GroupLayer", "Cannot remove layer %s from the group as it doesnt appear to be a child of the group", layer->m_LayerName.c_str());
}



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::tuple<LayerRecord, ChannelImageData> GroupLayer<T>::toPhotoshop(const Enum::ColorMode colorMode, const bool doCopy, const FileHeader& header)
{
	PascalString lrName = Layer<T>::generatePascalString();
	ChannelExtents extents = generateChannelExtents(ChannelCoordinates(Layer<T>::m_Width, Layer<T>::m_Height, Layer<T>::m_CenterX, Layer<T>::m_CenterY), header);
	uint16_t channelCount = static_cast<uint16_t>(Layer<T>::m_LayerMask.has_value());
	uint8_t clipping = 0u;	// No clipping mask for now
	LayerRecords::BitFlags bitFlags = LayerRecords::BitFlags(false, !Layer<T>::m_IsVisible, false);
	std::optional<LayerRecords::LayerMaskData> lrMaskData = Layer<T>::generateMaskData(header);
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

	auto blockVec = this->generateTaggedBlocks();
	std::optional<AdditionalLayerInfo> taggedBlocks = std::nullopt;
	if (blockVec.size() > 0)
	{
		TaggedBlockStorage blockStorage = { blockVec };
		taggedBlocks.emplace(blockStorage);
	}

	if (Layer<T>::m_BlendMode != Enum::BlendMode::Passthrough)
	{
		LayerRecord lrRecord = LayerRecord(
			lrName,
			extents.top,
			extents.left,
			extents.bottom,
			extents.right,
			channelCount,
			channelInfoVec,
			Layer<T>::m_BlendMode,
			Layer<T>::m_Opacity,
			clipping,
			bitFlags,
			lrMaskData,
			blendingRanges,
			std::move(taggedBlocks)
		);
		return std::make_tuple(std::move(lrRecord), ChannelImageData(std::move(channelDataVec)));
	}
	else
	{
		// If the group has a blendMode of Passthrough we actually need to pass that in the LrSectionDivider tagged block while the layers blendmode is set to normal
		LayerRecord lrRecord = LayerRecord(
			lrName,
			extents.top,
			extents.left,
			extents.bottom,
			extents.right,
			channelCount,
			channelInfoVec,
			Enum::BlendMode::Normal,
			Layer<T>::m_Opacity,
			clipping,
			bitFlags,
			lrMaskData,
			blendingRanges,
			std::move(taggedBlocks)
		);
		return std::make_tuple(std::move(lrRecord), ChannelImageData(std::move(channelDataVec)));
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
GroupLayer<T>::GroupLayer(const LayerRecord& layerRecord, ChannelImageData& channelImageData, const FileHeader& header) : Layer<T>(layerRecord, channelImageData, header)
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
GroupLayer<T>::GroupLayer(const Layer<T>::Params& layerParameters, bool isCollapsed /*= false*/)
{
	PROFILE_FUNCTION();
	Layer<T>::m_LayerName = layerParameters.layerName;
	Layer<T>::m_BlendMode = layerParameters.blendMode;
	Layer<T>::m_Opacity = layerParameters.opacity;
	Layer<T>::m_IsVisible = true;
	Layer<T>::m_CenterX = layerParameters.posX;
	Layer<T>::m_CenterY = layerParameters.posY;
	Layer<T>::m_Width = layerParameters.width;
	Layer<T>::m_Height = layerParameters.height;


	// Set the layer mask if present
	if (layerParameters.layerMask.has_value())
	{
		LayerMask<T> mask{};
		Enum::ChannelIDInfo info{ .id = Enum::ChannelID::UserSuppliedLayerMask, .index = -2 };
		ImageChannel<T> maskChannel = ImageChannel<T>(layerParameters.compression, std::move(layerParameters.layerMask.value()), info, layerParameters.width, layerParameters.height, layerParameters.posX, layerParameters.posY);
		mask.maskData = std::move(maskChannel);
		Layer<T>::m_LayerMask = mask;
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<std::shared_ptr<TaggedBlock>> GroupLayer<T>::generateTaggedBlocks()
{
	auto blockVec = Layer<T>::generateTaggedBlocks();

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
	blockVec.push_back(std::make_shared<LrSectionTaggedBlock>(sectionBlock));

	return blockVec;
}

PSAPI_NAMESPACE_END