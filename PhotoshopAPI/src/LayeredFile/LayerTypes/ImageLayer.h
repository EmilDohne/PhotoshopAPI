#pragma once

#include "Macros.h"
#include "Util/Enum.h"
#include "Layer.h"
#include "ImageDataMixins.h"

#include "Core/Struct/ImageChannel.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "LayeredFile/concepts.h"


#include <vector>
#include <unordered_map>
#include <optional>
#include <iostream>
#include <span>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>


PSAPI_NAMESPACE_BEGIN


template <typename T>
	requires concepts::bit_depth<T>
struct ImageLayer final : public Layer<T>, public WritableImageDataMixin<T>
{
	using typename Layer<T>::value_type;
	using typename WritableImageDataMixin<T>::data_type;
	using typename WritableImageDataMixin<T>::channel_type;
	using typename WritableImageDataMixin<T>::image_type;

public:

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	std::vector<int> channel_indices(bool include_mask) const
	{
		std::vector<int> indices{};
		for (const auto& [key, _] : WritableImageDataMixin<T>::m_ImageData)
		{
			indices.push_back(key.index);
		}
		if (Layer<T>::has_mask() && include_mask)
		{
			indices.push_back(Layer<T>::s_mask_index.index);
		}
		return indices;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	size_t num_channels(bool include_mask) const override
	{
		if (Layer<T>::has_mask() && include_mask)
		{
			return WritableImageDataMixin<T>::m_ImageData.size() + 1;
		}
		return WritableImageDataMixin<T>::m_ImageData.size();
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void set_write_compression(Enum::Compression _compcode) override
	{
		for (const auto& [_, channel_ptr] : WritableImageDataMixin<T>::m_ImageData)
		{
			channel_ptr->m_Compression = _compcode;
		}
		Layer<T>::set_mask_compression(_compcode);
	}

	/// Generate an ImageLayer instance ready to be used in a LayeredFile document. 
	/// 
	/// \param data the ImageData to associate with the layer
	/// \param parameters The parameters dictating layer name, width, height, mask etc.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	ImageLayer(std::unordered_map<Enum::ChannelID, std::vector<T>> data, Layer<T>::Params& parameters)
	{
		data_type remapped{};
		for (auto& [key, value] : data)
		{
			Enum::ChannelIDInfo info = Enum::toChannelIDInfo(key, parameters.colormode);
			remapped[info.index] = std::move(value);
		}
		construct(std::move(remapped), parameters);
	}

	/// Generate an ImageLayer instance ready to be used in a LayeredFile document.
	/// 
	/// \param imageData the ImageData to associate with the channel
	/// \param layerParameters The parameters dictating layer name, width, height, mask etc.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	ImageLayer(std::unordered_map<int, std::vector<T>> data, Layer<T>::Params& parameters) 
	{
		this->construct(std::move(data), parameters);
	}

	/// Initialize the ImageLayer from the photoshop primitives
	///
	/// This is part of the internal API and as a user you will likely never have to use 
	/// this function
	//
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	ImageLayer(const LayerRecord& layer_record, ChannelImageData& channel_image_data, const FileHeader& header)
		: Layer<T>(layer_record, channel_image_data, header)
	{
		// Move the layers into our own layer representation
		for (int i = 0; i < layer_record.m_ChannelCount; ++i)
		{
			auto& channel_info = layer_record.m_ChannelInformation[i];

			// We already extract masks ahead of time in ctor of Layer<T> and skip them here to avoid raising warnings
			if (channel_info.m_ChannelID.id == Enum::ChannelID::UserSuppliedLayerMask) continue;

			auto channelPtr = channel_image_data.extractImagePtr(channel_info.m_ChannelID);
			// Pointers might have already been released previously
			if (!channelPtr) continue;

			// Insert any valid pointers to channels we have. We move to avoid having 
			// to uncompress / recompress
			WritableImageDataMixin<T>::m_ImageData[channel_info.m_ChannelID] = std::move(channelPtr);
		}
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void set_image_data(const data_type& data) override
	{
		WritableImageDataMixin<T>::impl_set_image_data(
			data, 
			Layer<T>::m_Width, 
			Layer<T>::m_Height, 
			Layer<T>::m_CenterX, 
			Layer<T>::m_CenterY, 
			Layer<T>::m_ColorMode
		);
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void set_image_data(const data_type& data, int32_t width, int32_t height) override
	{
		WritableImageDataMixin<T>::impl_set_image_data(
			data,
			width,
			height,
			Layer<T>::m_CenterX,
			Layer<T>::m_CenterY,
			Layer<T>::m_ColorMode
		);
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void set_image_data(const std::unordered_map<Enum::ChannelID, std::vector<T>>& data) override
	{
		WritableImageDataMixin<T>::impl_set_image_data(
			data,
			Layer<T>::m_Width,
			Layer<T>::m_Height,
			Layer<T>::m_CenterX,
			Layer<T>::m_CenterY,
			Layer<T>::m_ColorMode
		);
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void set_image_data(const std::unordered_map<Enum::ChannelID, std::vector<T>>& data, int32_t width, int32_t height) override
	{
		WritableImageDataMixin<T>::impl_set_image_data(
			data,
			width,
			height,
			Layer<T>::m_CenterX,
			Layer<T>::m_CenterY,
			Layer<T>::m_ColorMode
		);
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void set_image_data(const std::unordered_map<Enum::ChannelIDInfo, std::vector<T>>& data) override
	{
		WritableImageDataMixin<T>::impl_set_image_data(
			data,
			Layer<T>::m_Width,
			Layer<T>::m_Height,
			Layer<T>::m_CenterX,
			Layer<T>::m_CenterY,
			Layer<T>::m_ColorMode
		);
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void set_image_data(const std::unordered_map<Enum::ChannelIDInfo, std::vector<T>>& data, int32_t width, int32_t height) override
	{
		WritableImageDataMixin<T>::impl_set_image_data(
			data,
			width,
			height,
			Layer<T>::m_CenterX,
			Layer<T>::m_CenterY,
			Layer<T>::m_ColorMode
		);
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void set_channel(int _id, const std::vector<T>& channel) override
	{
		WritableImageDataMixin<T>::impl_set_channel(
			WritableImageDataMixin<T>::idinfo_from_variant(_id, Layer<T>::m_ColorMode),
			std::span<const T>(channel.begin(), channel.end()),
			Layer<T>::m_Width,
			Layer<T>::m_Height,
			Layer<T>::m_CenterX,
			Layer<T>::m_CenterY,
			Layer<T>::m_ColorMode
		);
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void set_channel(Enum::ChannelID _id, const std::vector<T>& channel) override
	{
		WritableImageDataMixin<T>::impl_set_channel(
			WritableImageDataMixin<T>::idinfo_from_variant(_id, Layer<T>::m_ColorMode),
			std::span<const T>(channel.begin(), channel.end()),
			Layer<T>::m_Width,
			Layer<T>::m_Height,
			Layer<T>::m_CenterX,
			Layer<T>::m_CenterY,
			Layer<T>::m_ColorMode
		);
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void set_channel(Enum::ChannelIDInfo _id, const std::vector<T>& channel) override
	{
		WritableImageDataMixin<T>::impl_set_channel(
			WritableImageDataMixin<T>::idinfo_from_variant(_id, Layer<T>::m_ColorMode),
			std::span<const T>(channel.begin(), channel.end()),
			Layer<T>::m_Width,
			Layer<T>::m_Height,
			Layer<T>::m_CenterX,
			Layer<T>::m_CenterY,
			Layer<T>::m_ColorMode
		);
	}


	/// \brief Converts the image layer to Photoshop layerRecords and imageData.
	/// 
	/// This is part of the internal API and as a user you will likely never have to use 
	/// this function
	/// 
	/// \return A tuple containing LayerRecord and ChannelImageData.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	std::tuple<LayerRecord, ChannelImageData> to_photoshop() override
	{
		PascalString name = Layer<T>::generate_name();
		ChannelExtents extents = generate_extents(ChannelCoordinates(Layer<T>::m_Width, Layer<T>::m_Height, Layer<T>::m_CenterX, Layer<T>::m_CenterY));

		uint8_t clipping = 0u;	// No clipping mask for now
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
		auto channel_data = this->generate_channel_image_data();
		auto& channel_info = std::get<0>(channel_data);
		ChannelImageData channel_img_data = std::move(std::get<1>(channel_data));

		LayerRecord lr_record = LayerRecord(
			name,
			extents.top,
			extents.left,
			extents.bottom,
			extents.right,
			static_cast<uint16_t>(this->num_channels(true)),
			channel_info,
			Layer<T>::m_BlendMode,
			Layer<T>::m_Opacity,
			clipping,
			bit_flags,
			lr_mask_data,
			blending_ranges,
			std::move(tagged_blocks)
		);
		return std::make_tuple(std::move(lr_record), std::move(channel_img_data));
	}

protected:

	/// Extracts the m_ImageData as well as the layer mask into two vectors holding channel information as well as the image data 
	/// itself. This also takes care of generating our layer mask channel if it is present. Invalidates any data held by the ImageLayer
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	std::tuple<std::vector<LayerRecords::ChannelInformation>, ChannelImageData> generate_channel_image_data()
	{
		std::vector<LayerRecords::ChannelInformation> channel_info;
		std::vector<std::unique_ptr<ImageChannel>> channel_data;

		// First extract our mask data, the order of our channels does not matter as long as the 
		// order of channelInfo and channelData is the same
		auto mask_data = Layer<T>::internal_extract_mask();
		if (mask_data.has_value())
		{
			channel_info.push_back(std::get<0>(mask_data.value()));
			channel_data.push_back(std::move(std::get<1>(mask_data.value())));
		}

		// Extract all the channels next and push them into our data representation
		for (auto& [id, channel] : WritableImageDataMixin<T>::m_ImageData)
		{
			channel_info.push_back(LayerRecords::ChannelInformation{ id, channel->m_OrigByteSize });
			channel_data.push_back(std::move(channel));
		}

		// Construct the channel image data from our vector of ptrs, moving gets handled by the constructor
		ChannelImageData channel_image_data(std::move(channel_data));
		return std::make_tuple(channel_info, std::move(channel_image_data));
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	data_type evaluate_image_data() override
	{
		size_t num_channels_no_mask = this->num_channels(false);
		auto _channel_indices = this->channel_indices(false);

		if (num_channels_no_mask == 0)
		{
			throw std::runtime_error(fmt::format("ImageLayer '{}': Unable to evaluate image data without any channels present.", Layer<T>::m_LayerName));
		}
		if (!WritableImageDataMixin<T>::validate_channel_sizes())
		{
			throw std::runtime_error(fmt::format("ImageLayer '{}': Not all channels in the ImageLayer are the same size, unable to evaluate image data", Layer<T>::m_LayerName));
		}
		auto channel_size = WritableImageDataMixin<T>::m_ImageData.begin()->second->m_OrigByteSize / sizeof(T);

		size_t num_threads = std::thread::hardware_concurrency() / WritableImageDataMixin<T>::m_ImageData.size();
		num_threads = std::min(static_cast<size_t>(1), num_threads);

		// Allocate image data and then fill it by decompressing in parallel
		data_type data = WritableImageDataMixin<T>::parallel_alloc_image_data(_channel_indices, channel_size);
		std::for_each(std::execution::par_unseq, data.begin(), data.end(), [&](auto& pair)
			{
				auto& [key, channel_buffer] = pair;
				auto idinfo = Enum::toChannelIDInfo(key, Layer<T>::m_ColorMode);
				auto buffer_span = std::span<T>(channel_buffer.begin(), channel_buffer.end());
				WritableImageDataMixin<T>::m_ImageData[idinfo]->template getData<T>(buffer_span, num_threads);
			});

		if (Layer<T>::has_mask())
		{
			data[Layer<T>::s_mask_index.index] = Layer<T>::get_mask();
		}

		return std::move(data);
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	std::vector<T> evaluate_channel(std::variant<int, Enum::ChannelID, Enum::ChannelIDInfo> _id) override
	{
		auto idinfo = WritableImageDataMixin<T>::idinfo_from_variant(_id, Layer<T>::m_ColorMode);
		// short-circuit masks
		if (idinfo == Layer<T>::s_mask_index && Layer<T>::has_mask())
		{
			return Layer<T>::get_mask();
		}

		if (!WritableImageDataMixin<T>::m_ImageData.contains(idinfo))
		{
			throw std::invalid_argument(fmt::format("ImageLayer '{}': Invalid channel '{}' accessed while calling evaluate_channel()", Layer<T>::m_LayerName, Enum::channelIDToString(idinfo.id)));
		}
		return WritableImageDataMixin<T>::m_ImageData.at(idinfo)->template getData<T>();
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void impl_set_mask(const std::span<const T> data, int32_t width, int32_t height, float center_x, float center_y)
	{
		Layer<T>::set_mask(data, width, height);
		Layer<T>::mask_position(Geometry::Point2D<double>(center_x, center_y));
	}

private:

	/// Construct and initialize the layer from memory.
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void construct(data_type data, Layer<T>::Params& parameters)
	{
		PSAPI_PROFILE_FUNCTION();
		Layer<T>::m_ColorMode = parameters.colormode;
		Layer<T>::m_LayerName = parameters.name;
		if (parameters.blendmode == Enum::BlendMode::Passthrough)
		{
			PSAPI_LOG_WARNING("ImageLayer", "The Passthrough blend mode is reserved for groups, defaulting to 'Normal'");
			Layer<T>::m_BlendMode = Enum::BlendMode::Normal;
		}
		else
		{
			Layer<T>::m_BlendMode = parameters.blendmode;
		}
		Layer<T>::m_Opacity = parameters.opacity;
		Layer<T>::m_IsVisible = parameters.visible;
		Layer<T>::m_IsLocked = parameters.locked;
		Layer<T>::m_CenterX = static_cast<float>(parameters.center_x);
		Layer<T>::m_CenterY = static_cast<float>(parameters.center_y);
		Layer<T>::m_Width = parameters.width;
		Layer<T>::m_Height = parameters.height;

		// Forward the mask channel if it was passed as part of the image data to the layer mask
		// The actual populating of the mask channel will be done further down
		if (data.contains(Layer<T>::s_mask_index.index))
		{
			if (parameters.mask)
			{
				PSAPI_LOG_ERROR("ImageLayer",
					"Got mask from both the ImageData as index -2 and as part of the layer parameter, please only pass it as one of these");
			}

			PSAPI_LOG_DEBUG("ImageLayer", "Forwarding mask channel passed as part of image data to m_LayerMask");
			parameters.mask = std::move(data[Layer<T>::s_mask_index.index]);
			data.erase(Layer<T>::s_mask_index.index);
		}


		// Apply the image data and mask channel.
		WritableImageDataMixin<T>::impl_set_image_data(
			data, 
			Layer<T>::m_Width, 
			Layer<T>::m_Height, 
			Layer<T>::m_CenterX, 
			Layer<T>::m_CenterY, 
			Layer<T>::m_ColorMode
		);
		Layer<T>::parse_mask(parameters);

		// Do a check if the channels contain the minimum required for the given colormode.
		if (!WritableImageDataMixin<T>::validate_channels(Layer<T>::m_ColorMode))
		{
			throw std::runtime_error(fmt::format("ImageLayer '{}': Invalid channels passed to constructor", Layer<T>::m_LayerName));
		}
	}

};


extern template struct ImageLayer<bpp8_t>;
extern template struct ImageLayer<bpp16_t>;
extern template struct ImageLayer<bpp32_t>;


PSAPI_NAMESPACE_END