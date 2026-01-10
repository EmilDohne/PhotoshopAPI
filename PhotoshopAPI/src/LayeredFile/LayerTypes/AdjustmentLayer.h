#pragma once

#include "Macros.h"
#include "Layer.h"
#include "LayeredFile/concepts.h"

PSAPI_NAMESPACE_BEGIN

/// This struct holds no data, we just use it to identify its type
/// This will probably be split into multiple files later on
template <typename T>
	requires concepts::bit_depth<T>
struct AdjustmentLayer : Layer<T>
{
	using Layer<T>::Layer;
	AdjustmentLayer() = default;
	AdjustmentLayer(const LayerRecord& layer_record, ChannelImageData& channel_image_data, const FileHeader& header)
		: Layer<T>(layer_record, channel_image_data, header)
	{
		// Move the layers into our own layer representation
		for (size_t i = 0; i < layer_record.m_ChannelCount; ++i)
		{
			auto& channel_info = layer_record.m_ChannelInformation[i];

			// We already extract masks ahead of time in ctor of Layer<T> and skip them here to avoid raising warnings
			if (channel_info.m_ChannelID.id == Enum::ChannelID::UserSuppliedLayerMask) continue;

			auto channelPtr = channel_image_data.extract_image_ptr(channel_info.m_ChannelID);
			// Pointers might have already been released previously
			if (!channelPtr) continue;

			// Insert any valid pointers to channels we have. We move to avoid having 
			// to uncompress / recompress
			Layer<T>::m_UnparsedImageData[channel_info.m_ChannelID] = std::move(channelPtr);
		}
	}

	size_t num_channels(bool include_mask) const
	{
		if (Layer<T>::has_mask() && include_mask)
		{
			return Layer<T>::m_UnparsedImageData.size() + 1;
		}
		return Layer<T>::m_UnparsedImageData.size();
	}

	std::tuple<LayerRecord, ChannelImageData> to_photoshop() override
	{
		PascalString name = Layer<T>::generate_name();
		ChannelExtents extents = generate_extents(ChannelCoordinates(Layer<T>::m_Width, Layer<T>::m_Height, Layer<T>::m_CenterX, Layer<T>::m_CenterY));

		LayerRecords::BitFlags bit_flags(Layer<T>::m_IsLocked, !Layer<T>::m_IsVisible, false);
		std::optional<LayerRecords::LayerMaskData> lr_mask_data = Layer<T>::internal_generate_mask_data();
		LayerRecords::LayerBlendingRanges blending_ranges = Layer<T>::generate_blending_ranges();

		// Generate our AdditionalLayerInfoSection. We dont need any special Tagged Blocks besides what is stored by the generic layer
		auto block_vec = this->generate_tagged_blocks();
		std::optional<AdditionalLayerInfo> tagged_blocks = std::nullopt;
		if (block_vec.size() > 0)
		{
			TaggedBlockStorage blockStorage = { block_vec };
			tagged_blocks.emplace(AdditionalLayerInfo(blockStorage));
		}

		// Initialize the channel information as well as the channel image data, the size held in the channelInfo might change depending on
		// the compression mode chosen on export and must therefore be updated later. This step is done last as generateChannelImageData() invalidates
		// all image data which we might need for operations above
		auto num_channels = this->num_channels(true);

		auto channel_data = this->generate_channel_image_data();
		auto& channel_info = std::get<0>(channel_data);
		ChannelImageData channel_img_data = std::move(std::get<1>(channel_data));

		LayerRecord lr_record = LayerRecord(
			name,
			extents.top,
			extents.left,
			extents.bottom,
			extents.right,
			static_cast<uint16_t>(num_channels),
			channel_info,
			Layer<T>::m_BlendMode,
			Layer<T>::m_Opacity,
			static_cast<uint8_t>(Layer<T>::m_IsClippingMask),
			bit_flags,
			lr_mask_data,
			blending_ranges,
			std::move(tagged_blocks)
		);
		return std::make_tuple(std::move(lr_record), std::move(channel_img_data));
	}

	std::tuple<std::vector<LayerRecords::ChannelInformation>, ChannelImageData> generate_channel_image_data()
	{
		std::vector<LayerRecords::ChannelInformation> channel_info;
		std::vector<std::unique_ptr<channel_wrapper>> channel_data;

		// First extract our mask data, the order of our channels does not matter as long as the 
		// order of channelInfo and channelData is the same
		auto mask_data = Layer<T>::internal_extract_mask();
		if (mask_data.has_value())
		{
			channel_info.push_back(std::get<0>(mask_data.value()));
			channel_data.push_back(std::move(std::get<1>(mask_data.value())));
		}

		// Extract all the channels next and push them into our data representation
		for (auto& [id, channel] : Layer<T>::m_UnparsedImageData)
		{
			channel_info.push_back(LayerRecords::ChannelInformation{ id, channel->byte_size() });
			channel_data.push_back(std::move(channel));
		}

		// Construct the channel image data from our vector of ptrs, moving gets handled by the constructor
		ChannelImageData channel_image_data(std::move(channel_data));
		return std::make_tuple(channel_info, std::move(channel_image_data));
	}
};

extern template struct AdjustmentLayer<bpp8_t>;
extern template struct AdjustmentLayer<bpp16_t>;
extern template struct AdjustmentLayer<bpp32_t>;

PSAPI_NAMESPACE_END