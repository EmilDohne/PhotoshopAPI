#pragma once

#include "Macros.h"
#include "Layer.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "Core/TaggedBlocks/TaggedBlock.h"
#include "Core/TaggedBlocks/TaggedBlockStorage.h"
#include "LayeredFile/concepts.h"
#include "LayeredFile/fwd.h"
#include "LayeredFile/LayeredFile.h"

#include <vector>
#include <variant>
#include <memory>

PSAPI_NAMESPACE_BEGIN



/// \brief Represents a group of layers that may contain nested child layers.
/// 
/// \tparam T The data type for pixel values in layers (e.g., uint8_t, uint16_t, float32_t).
/// 
template <typename T>
	requires concepts::bit_depth<T>
struct GroupLayer final: public Layer<T>
{
	/// \defgroup layer The groups' child layers
	/// @{
	std::vector<std::shared_ptr<Layer<T>>>& layers() noexcept { return m_Layers; }
	const std::vector<std::shared_ptr<Layer<T>>>& layers() const noexcept { return m_Layers; }
	void layers(std::vector<std::shared_ptr<Layer<T>>> layer_vec) noexcept { m_Layers = std::move(layer_vec); }
	/// @}

	/// \defgroup collapse Whether the group is collapsed
	/// @{
	bool& collapsed() noexcept { return m_isCollapsed; }
	bool collapsed() const noexcept { return m_isCollapsed; }
	void collapsed(bool is_collapsed) noexcept { m_isCollapsed = is_collapsed; }
	/// @} 

	/// \brief Constructs a GroupLayer with the given layer parameters and collapse state.
	/// \param parameters The parameters for the group layer.
	/// \param isCollapsed Specifies whether the group layer is initially collapsed.
	GroupLayer(Layer<T>::Params& parameters, bool isCollapsed = false)
	{
		PSAPI_PROFILE_FUNCTION();
		Layer<T>::m_ColorMode = parameters.colormode;
		Layer<T>::m_LayerName = parameters.name;
		Layer<T>::m_BlendMode = parameters.blendmode;
		Layer<T>::m_Opacity = parameters.opacity;
		Layer<T>::m_IsVisible = parameters.visible;
		Layer<T>::m_IsLocked = parameters.locked;
		Layer<T>::m_CenterX = static_cast<float>(parameters.center_x);
		Layer<T>::m_CenterY = static_cast<float>(parameters.center_y);
		Layer<T>::m_Width = parameters.width;
		Layer<T>::m_Height = parameters.height;
		Layer<T>::m_IsClippingMask = parameters.clipping_mask;

		if (Layer<T>::m_IsClippingMask)
		{
			throw std::invalid_argument("clipping masks are not supported on group layers");
		}

		m_isCollapsed = isCollapsed;

		Layer<T>::parse_mask(parameters);

		// If the mask was passed but not a width and height we throw an error 
		if (Layer<T>::has_mask() && Layer<T>::m_Width == 0 && Layer<T>::m_Height == 0)
		{
			PSAPI_LOG_ERROR("GroupLayer", "Mask parameter specified but width and height are not set to the masks' dimensions");
		}

		// Throw an error if the width and height are set but no mask is passed. This is technically not necessary as 
		// writing a file with width and height but no image data is a no-op but we want to enforce good practice
		if (!Layer<T>::has_mask() && (Layer<T>::m_Width > 0 || Layer<T>::m_Height > 0))
		{
			PSAPI_LOG_ERROR("GroupLayer", "Non-zero height or width passed but no mask specified. Got {width: %d, height: %d} but expected {0, 0}", 
				static_cast<int>(Layer<T>::m_Width), static_cast<int>(Layer<T>::m_Height));
		}
	}

	/// \brief Adds a layer to the group, checking for duplicates in the process.
	/// \param layeredFile The layered file containing the group.
	/// \param layer The layer to be added.
	void add_layer(const LayeredFile<T>& layeredFile, std::shared_ptr<Layer<T>> layer)
	{
		if (layeredFile.is_layer_in_file(layer))
		{
			PSAPI_LOG_WARNING("GroupLayer", "Cannot insert a layer into the document twice, please use a unique layer. Skipping layer '%s'", layer->name().c_str());
			return;
		}
		m_Layers.push_back(layer);
	}

	/// \brief Removes a layer at the given index from the group.
	/// \param index The index of the layer to be removed.
	void remove_layer(const int index)
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
	void remove_layer(std::shared_ptr<Layer<T>>& layer)
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
		PSAPI_LOG_WARNING("GroupLayer", "Cannot remove layer %s from the group as it doesnt appear to be a child of the group", layer->name().c_str());
	}

	/// \brief Removes the specified layer from the group.
	/// \param layerName The name of the layer to be removed
	void remove_layer(const std::string layerName)
	{
		int index = 0;
		for (auto& sceneLayer : m_Layers)
		{
			if (layerName == sceneLayer->name())
			{
				m_Layers.erase(m_Layers.begin() + index);
				return;
			}
			++index;
		}
		PSAPI_LOG_WARNING("GroupLayer", "Cannot remove layer %s from the group as it doesnt appear to be a child of the group", layerName.c_str());
	}

	/// \brief Converts the group layer to Photoshop layerRecords and imageData.
	/// 
	/// This is part of the internal API and as a user you will likely never have to use 
	/// this function
	/// 
	/// \return A tuple containing layerRecords and imageData.
	std::tuple<LayerRecord, ChannelImageData> to_photoshop() override
	{
		PascalString lrName = Layer<T>::generate_name();
		ChannelExtents extents = generate_extents(ChannelCoordinates(Layer<T>::m_Width, Layer<T>::m_Height, Layer<T>::m_CenterX, Layer<T>::m_CenterY));
		LayerRecords::BitFlags bitFlags = LayerRecords::BitFlags(Layer<T>::m_IsLocked, !Layer<T>::m_IsVisible, false);
		std::optional<LayerRecords::LayerMaskData> lrMaskData = Layer<T>::internal_generate_mask_data();
		LayerRecords::LayerBlendingRanges blendingRanges = Layer<T>::generate_blending_ranges();


		// Initialize the channelInfo. Note that if the data is to be compressed the channel size gets update
		// again later
		std::vector<LayerRecords::ChannelInformation> channelInfoVec;
		std::vector<std::unique_ptr<ImageChannel>> channelDataVec;

		// First extract our mask data, the order of our channels does not matter as long as the 
		// order of channelInfo and channelData is the same
		auto maskData = Layer<T>::internal_extract_mask();
		if (maskData.has_value())
		{
			channelInfoVec.push_back(std::get<0>(maskData.value()));
			channelDataVec.push_back(std::move(std::get<1>(maskData.value())));
		}

		auto blockVec = this->generate_tagged_blocks();
		std::optional<AdditionalLayerInfo> taggedBlocks = std::nullopt;
		if (blockVec.size() > 0)
		{
			TaggedBlockStorage blockStorage = { blockVec };
			taggedBlocks.emplace(blockStorage);
		}

		// Applications such as krita expect empty channels to be in-place for the given colormode
		// to actually parse the file. 
		Layer<T>::generate_empty_channels(channelInfoVec, channelDataVec, Layer<T>::m_ColorMode);

		if (Layer<T>::m_BlendMode != Enum::BlendMode::Passthrough)
		{
			LayerRecord lrRecord = LayerRecord(
				lrName,
				extents.top,
				extents.left,
				extents.bottom,
				extents.right,
				static_cast<uint16_t>(channelInfoVec.size()),
				channelInfoVec,
				Layer<T>::m_BlendMode,
				Layer<T>::m_Opacity,
				static_cast<uint8_t>(Layer<T>::m_IsClippingMask),
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
				static_cast<uint16_t>(channelInfoVec.size()),
				channelInfoVec,
				Enum::BlendMode::Normal,
				Layer<T>::m_Opacity,
				static_cast<uint8_t>(Layer<T>::m_IsClippingMask),
				bitFlags,
				lrMaskData,
				blendingRanges,
				std::move(taggedBlocks)
			);
			return std::make_tuple(std::move(lrRecord), ChannelImageData(std::move(channelDataVec)));
		}
	}

	/// \brief Constructs a GroupLayer using layerRecord, channelImageData, and file header.
	/// 
	/// This is part of the internal API and as a user you will likely never have to use 
	/// this function
	/// 
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


protected:

	/// Child layers contained within the group. Note that Layer<T> is polymorphic.
	std::vector<std::shared_ptr<Layer<T>>> m_Layers;

	/// Specifies whether or not the layer is collapsed or open
	bool m_isCollapsed = false;

	/// \brief Generate the tagged blocks necessary for writing the layer
	std::vector<std::shared_ptr<TaggedBlock>> generate_tagged_blocks() override
	{
		auto blockVec = Layer<T>::generate_tagged_blocks();
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

extern template struct GroupLayer<bpp8_t>;
extern template struct GroupLayer<bpp16_t>;
extern template struct GroupLayer<bpp32_t>;


PSAPI_NAMESPACE_END