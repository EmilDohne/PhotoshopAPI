#pragma once

#include "Macros.h"
#include "Util/Enum.h"
#include "Layer.h"
#include "_ImageDataLayerType.h"

#include "Core/Struct/ImageChannel.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"


#include <vector>
#include <unordered_map>
#include <optional>
#include <iostream>

// If we compile with C++<20 we replace the stdlib implementation with the compatibility
// library
#if (__cplusplus < 202002L)
#include "tcb_span.hpp"
#else
#include <span>
#endif

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>


PSAPI_NAMESPACE_BEGIN


// A pixel based image layer, the image data is stored on the ImageDataLayerType
template <typename T>
struct ImageLayer : public _ImageDataLayerType<T>
{
	using _ImageDataLayerType<T>::data_type;
	using _ImageDataLayerType<T>::storage_type;

	/// Generate an ImageLayer instance ready to be used in a LayeredFile document. 
	/// 
	/// \tparam ExecutionPolicy the execution policy to generate the image layer with, at most parallelizes to data.size()
	/// 
	/// \param data the ImageData to associate with the layer
	/// \param parameters The parameters dictating layer name, width, height, mask etc.
	/// \param policy The execution policy for the image data compression
	template <typename  ExecutionPolicy = std::execution::parallel_policy, std::enable_if_t<std::is_execution_policy_v<ExecutionPolicy>, int> = 0>
	ImageLayer(std::unordered_map<Enum::ChannelID, std::vector<T>>&& data, Layer<T>::Params& parameters, const ExecutionPolicy policy = std::execution::par)
	{
		// Change the data from being Enum::ChannelID mapped to instead being mapped to a ChannelIDInfo so we can forward it
		// to the other constructor
		typename _ImageDataLayerType<T>::data_type remapped{};
		for (auto& [key, value] : data)
		{
			// Check in a strict manner whether the channel is even valid for the colormode
			if (!Enum::channelValidForColorMode(key, parameters.colormode))
			{
				PSAPI_LOG_WARNING("ImageLayer", "Unable to construct channel '%s' as it is not valid for the '%s' colormode. Skipping creation of this channel",
					Enum::channelIDToString(key).c_str(), Enum::colorModeToString(parameters.colormode).c_str());
				continue;
			}
			Enum::ChannelIDInfo info = Enum::toChannelIDInfo(key, parameters.colormode);
			remapped[info] = std::move(value);
		}
		_ImageDataLayerType<T>::construct(std::move(remapped), parameters);
	}

	/// Generate an ImageLayer instance ready to be used in a LayeredFile document.
	/// 
	/// \param imageData the ImageData to associate with the channel
	/// \param layerParameters The parameters dictating layer name, width, height, mask etc.
	ImageLayer(std::unordered_map<int16_t, std::vector<T>>&& data, Layer<T>::Params& parameters) 
	{
		// Change the data from being int16_t mapped to instead being mapped to a ChannelIDInfo so we can forward it
		// to construct
		typename _ImageDataLayerType<T>::data_type remapped{};
		for (auto& [key, value] : data)
		{
			// Check in a strict manner whether the channel is even valid for the colormode
			if (!Enum::channelValidForColorMode(key, parameters.colormode))
			{
				PSAPI_LOG_WARNING("ImageLayer", "Unable to construct channel with index %d as it is not valid for the '%s' colormode. Skipping creation of this channel",
					key, Enum::colorModeToString(parameters.colormode).c_str());
				continue;
			}
			Enum::ChannelIDInfo info = Enum::toChannelIDInfo(key, parameters.colormode);
			remapped[info] = std::move(value);
		}
		_ImageDataLayerType<T>::construct(std::move(remapped), parameters);
	}

	/// Initialize our imageLayer by first parsing the base Layer instance and then moving
	/// the additional channels into our representation. This constructor is primarily for internal usage and it is 
	/// encouraged to use the other constructors taking image data directly instead.
	ImageLayer(const LayerRecord& layerRecord, ChannelImageData& channelImageData, const FileHeader& header) :
		_ImageDataLayerType<T>(layerRecord, channelImageData, header)
	{
		// Move the layers into our own layer representation
		for (int i = 0; i < layerRecord.m_ChannelCount; ++i)
		{
			auto& channelInfo = layerRecord.m_ChannelInformation[i];

			// We already extract masks ahead of time and skip them here to avoid raising warnings
			if (channelInfo.m_ChannelID.id == Enum::ChannelID::UserSuppliedLayerMask) continue;

			auto channelPtr = channelImageData.extractImagePtr(channelInfo.m_ChannelID);
			// Pointers might have already been released previously
			if (!channelPtr) continue;

			// Insert any valid pointers to channels we have. We move to avoid having 
			// to uncompress / recompress
			_ImageDataLayerType<T>::m_ImageData[channelInfo.m_ChannelID] = std::move(channelPtr);
		}
	}

	/// Change the compression codec of all the image channels. This applies
	/// on-write
	void set_compression(const Enum::Compression compCode) override
	{
		// Change the mask channels' compression codec
		Layer<T>::set_compression(compCode);
		// Change the image channel compression codecs
		for (const auto& [key, val] : _ImageDataLayerType<T>::m_ImageData)
		{
			_ImageDataLayerType<T>::m_ImageData[key]->m_Compression = compCode;
		}
	}

	/// \brief Converts the image layer to Photoshop layerRecords and imageData.
	/// 
	/// This is part of the internal API and as a user you will likely never have to use 
	/// this function
	/// 
	/// \param colorMode The color mode for the conversion.
	/// \param header The file header for the conversion.
	/// \return A tuple containing layerRecords and imageData.
	std::tuple<LayerRecord, ChannelImageData> to_photoshop(const Enum::ColorMode colorMode, const FileHeader& header) override
	{
		PascalString lrName = Layer<T>::generate_name();
		ChannelExtents extents = generate_extents(ChannelCoordinates(Layer<T>::m_Width, Layer<T>::m_Height, Layer<T>::m_CenterX, Layer<T>::m_CenterY), header);
		uint16_t channelCount = _ImageDataLayerType<T>::m_ImageData.size() + static_cast<uint16_t>(Layer<T>::m_LayerMask.has_value());

		uint8_t clipping = 0u;	// No clipping mask for now
		LayerRecords::BitFlags bitFlags(Layer<T>::m_IsLocked, !Layer<T>::m_IsVisible, false);
		std::optional<LayerRecords::LayerMaskData> lrMaskData = Layer<T>::generate_mask(header);
		LayerRecords::LayerBlendingRanges blendingRanges = Layer<T>::generate_blending_ranges();

		// Generate our AdditionalLayerInfoSection. We dont need any special Tagged Blocks besides what is stored by the generic layer
		auto blockVec = this->generate_tagged_blocks();
		std::optional<AdditionalLayerInfo> taggedBlocks = std::nullopt;
		if (blockVec.size() > 0)
		{
			TaggedBlockStorage blockStorage = { blockVec };
			taggedBlocks.emplace(blockStorage);
		}

		// Initialize the channel information as well as the channel image data, the size held in the channelInfo might change depending on
		// the compression mode chosen on export and must therefore be updated later. This step is done last as generateChannelImageData() invalidates
		// all image data which we might need for operations above
		auto channelData = _ImageDataLayerType<T>::generate_channel_image_data();
		auto& channelInfoVec = std::get<0>(channelData);
		ChannelImageData channelImgData = std::move(std::get<1>(channelData));

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

		return std::make_tuple(std::move(lrRecord), std::move(channelImgData));
	}
};

PSAPI_NAMESPACE_END