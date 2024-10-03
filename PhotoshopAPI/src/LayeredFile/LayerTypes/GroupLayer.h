#pragma once

#include "Macros.h"
#include "Layer.h"
#include "ImageLayer.h"
#include "SectionDividerLayer.h"
#include "ArtboardLayer.h"
#include "ShapeLayer.h"
#include "SmartObjectLayer.h"
#include "TextLayer.h"
#include "AdjustmentLayer.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "Core/Struct/TaggedBlock.h"
#include "Core/Struct/TaggedBlockStorage.h"
#include "LayeredFile/LayeredFile.h"

#include <vector>
#include <variant>
#include <memory>

PSAPI_NAMESPACE_BEGIN

// Forward declare LayeredFile here
template <typename T>
struct LayeredFile;


/// \brief Represents a group of layers that may contain nested child layers.
/// 
/// \tparam T The data type for pixel values in layers (e.g., uint8_t, uint16_t, float32_t).
/// 
template <typename T>
struct GroupLayer : public Layer<T>
{
	/// Child layers contained within the group. Note that Layer<T> is polymorphic.
	std::vector<std::shared_ptr<Layer<T>>> m_Layers;

	/// Specifies whether or not the layer is collapsed or open
	bool m_isCollapsed = false;		

	/// \brief Adds a layer to the group, checking for duplicates in the process.
	/// \param layeredFile The layered file containing the group.
	/// \param layer The layer to be added.
	void addLayer(const LayeredFile<T>& layeredFile, std::shared_ptr<Layer<T>> layer)
	{
		if (layeredFile.isLayerInDocument(layer))
		{
			PSAPI_LOG_WARNING("GroupLayer", "Cannot insert a layer into the document twice, please use a unique layer. Skipping layer '%s'", layer->m_LayerName.c_str());
			return;
		}
		m_Layers.push_back(layer);
	}

	/// \brief Removes a layer at the given index from the group.
	/// \param index The index of the layer to be removed.
	void removeLayer(const int index)
	{
		if (index >= m_Layers.size())
		{
			PSAPI_LOG_WARNING("GroupLayer", "Cannot remove index %i from the group as it would exceed the amount of layers in the group", index);
			return;
		}
		m_Layers.erase(m_Layers.begin() + index);
	}

	/// \brief Removes the specified layer from the group.
	/// \param layer The layer to be removed.
	void removeLayer(std::shared_ptr<Layer<T>>& layer)
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

	/// \brief Removes the specified layer from the group.
	/// \param layerName The name of the layer to be removed
	void removeLayer(const std::string layerName)
	{
		int index = 0;
		for (auto& sceneLayer : m_Layers)
		{
			if (layerName == sceneLayer->m_LayerName)
			{
				m_Layers.erase(m_Layers.begin() + index);
				return;
			}
			++index;
		}
		PSAPI_LOG_WARNING("GroupLayer", "Cannot remove layer %s from the group as it doesnt appear to be a child of the group", layerName.c_str());
	}

	/// \brief Converts the group layer to Photoshop layerRecords and imageData.
	/// \param colorMode The color mode for the conversion.
	/// \param header The file header for the conversion.
	/// \return A tuple containing layerRecords and imageData.
	std::tuple<LayerRecord, ChannelImageData> toPhotoshop(const Enum::ColorMode colorMode, const FileHeader& header) override
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
		std::vector<std::unique_ptr<ImageChannel>> channelDataVec;

		// First extract our mask data, the order of our channels does not matter as long as the 
		// order of channelInfo and channelData is the same
		auto maskData = Layer<T>::extractLayerMask();
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

	/// \brief Constructs a GroupLayer using layerRecord, channelImageData, and file header.
	/// \param layerRecord The layer record for the group layer.
	/// \param channelImageData The channel image data for the group layer.
	/// \param header The file header for the group layer.
	GroupLayer(const LayerRecord& layerRecord, ChannelImageData& channelImageData, const FileHeader& header) : Layer<T>(layerRecord, channelImageData, header)
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

	/// \brief Constructs a GroupLayer with the given layer parameters and collapse state.
	/// \param layerParameters The parameters for the group layer.
	/// \param isCollapsed Specifies whether the group layer is initially collapsed.
	GroupLayer(Layer<T>::Params& parameters, bool isCollapsed = false)
	{
		PROFILE_FUNCTION();
		Layer<T>::m_LayerName = parameters.layerName;
		Layer<T>::m_BlendMode = parameters.blendMode;
		Layer<T>::m_Opacity = parameters.opacity;
		Layer<T>::m_IsVisible = parameters.isVisible;
		Layer<T>::m_CenterX = parameters.posX;
		Layer<T>::m_CenterY = parameters.posY;
		Layer<T>::m_Width = parameters.width;
		Layer<T>::m_Height = parameters.height;

		m_isCollapsed = isCollapsed;

		// Set the layer mask if present
		Layer<T>::parseLayerMask(parameters);
	}


protected:
	/// \brief Generate the tagged blocks necessary for writing the layer
	std::vector<std::shared_ptr<TaggedBlock>> generateTaggedBlocks() override
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
};



PSAPI_NAMESPACE_END