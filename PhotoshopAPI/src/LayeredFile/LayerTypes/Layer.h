#pragma once

#include "Macros.h"
#include "Util/Enum.h"
#include "Core/Struct/ImageChannel.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "PhotoshopFile/AdditionalLayerInfo.h"

#include "Core/TaggedBlocks/TaggedBlock.h"
#include "Core/TaggedBlocks/LinkedLayerTaggedBlock.h"
#include "Core/TaggedBlocks/LrSectionTaggedBlock.h"
#include "Core/TaggedBlocks/Lr16TaggedBlock.h"
#include "Core/TaggedBlocks/Lr32TaggedBlock.h"
#include "Core/TaggedBlocks/PlacedLayerTaggedBlock.h"
#include "Core/TaggedBlocks/ProtectedSettingTaggedBlock.h"
#include "Core/TaggedBlocks/ReferencePointTaggedBlock.h"
#include "Core/TaggedBlocks/UnicodeLayerNameTaggedBlock.h"

#include "MaskDataMixin.h"
#include "LayeredFile/concepts.h"


#include <vector>
#include <optional>
#include <string>
#include <memory>

PSAPI_NAMESPACE_BEGIN


/// Base Struct for Layers of all types (Group, Image, [Adjustment], etc.) which includes the minimum to parse a generic layer type
template <typename T>
	requires concepts::bit_depth<T>
struct Layer : public MaskMixin<T>
{
	/// Template type accessor which can be used using decltype(layer::value_type)
	using value_type = T;

	/// Layer Parameters for initialization of a generic layer type. It provides sensible defaults so only what is needed
	/// needs to be overridden
	struct Params
	{
		/// Optional Layer Mask parameter, if none is specified there is no mask. This image data must have the same size as 
		/// the layer itself
		std::optional<std::vector<T>> mask = std::nullopt;
		/// The Layer Name to give to the layer, has a maximum length of 255
		std::string name = "";
		/// The Layers Blend Mode, all available blend modes are valid except for 'Passthrough' on non-group layers
		Enum::BlendMode blendmode = Enum::BlendMode::Normal;
		/// The X Center coordinate in respect to the canvas' top left. So a value of 32 would mean the layer is centered 32 pixels from the left of the canvas.
		int32_t center_x = 0;
		/// The Y Center coordinate in respect to the canvas' top left. So a value of 32 would mean the layer is centered 32 pixels from the top of the canvas.
		int32_t center_y = 0;
		/// The width of the layer, this value must be passed explicitly as we do not deduce this from the Image Data itself
		uint32_t width = 0u;
		/// The height of the layer, this value must be passed explicitly as we do not deduce this from the Image Data itself
		uint32_t height = 0u;
		/// The Layer opacity, the value displayed by Photoshop will be this value / 255 so 255 corresponds to 100% 
		/// while 128 would correspond to ~50%
		uint8_t opacity = 255u;
		// The compression codec of the layer, it is perfectly valid for each layer (and channel) to be compressed differently
		Enum::Compression compression = Enum::Compression::ZipPrediction; 
		// The Layers color mode, currently only RGB is supported
		Enum::ColorMode colormode = Enum::ColorMode::RGB;
		// Whether the layer is visible
		bool visible = true;
		// Whether the layer is locked
		bool locked = false;
		// Whether the layer is clipped to the one below
		bool clipping_mask = false;
	};

	/// The layers' name. Stored as a utf-8 string
	const std::string& name() const noexcept { return m_LayerName; }

	/// The layers' name. Stored as a utf-8 string
	std::string& name() noexcept { return m_LayerName; }

	/// The layers' name. Stored as a utf-8 string
	void name(const std::string& layer_name) noexcept { m_LayerName = layer_name; }
	
	/// The blendmode of the layer, the `Passthrough` blendmode is only valid for groups
	Enum::BlendMode& blendmode() noexcept { return m_BlendMode; }
	/// The blendmode of the layer, the `Passthrough` blendmode is only valid for groups
	Enum::BlendMode blendmode() const noexcept { return m_BlendMode; }
	/// The blendmode of the layer, the `Passthrough` blendmode is only valid for groups
	void blendmode(Enum::BlendMode blend_mode) noexcept { m_BlendMode = blend_mode; }
	
	/// Whether the layers' pixel values are locked. This is currently an all or nothing setting
	bool& locked() noexcept { return m_IsLocked; }
	/// Whether the layers' pixel values are locked. This is currently an all or nothing setting
	bool locked() const noexcept { return m_IsLocked; }
	/// Whether the layers' pixel values are locked. This is currently an all or nothing setting
	void locked(bool is_locked) noexcept { m_IsLocked = is_locked; }

	/// Visibility toggle of the layer
	bool& visible() noexcept { return m_IsVisible; }
	/// Visibility toggle of the layer
	bool visible() const noexcept { return m_IsVisible; }
	/// Visibility toggle of the layer
	void visible(bool is_visible) noexcept { m_IsVisible = is_visible; }

	/// Clipping mask toggle of the layer, clips it to the layer below
	bool& clipping_mask() noexcept { return m_IsClippingMask; }
	/// Clipping mask toggle of the layer, clips it to the layer below
	bool clipping_mask() const noexcept { return m_IsClippingMask; }
	/// Clipping mask toggle of the layer, clips it to the layer below
	void clipping_mask(bool is_clipped) noexcept { m_IsClippingMask = is_clipped; }

	/// The layers' opacity. 
	/// 
	/// In photoshop this is stored as a `uint8_t` from 0-255 but access and write is 
	/// in terms of a float for better consistency.
	float opacity() const noexcept { return static_cast<float>(m_Opacity) / 255; }
	/// The layers' opacity. 
	/// 
	/// In photoshop this is stored as a `uint8_t` from 0-255 but access and write is 
	/// in terms of a float for better consistency.
	void opacity(float value) noexcept 
	{
		if (value < 0.0f || value > 1.0f)
		{
			PSAPI_LOG_WARNING("Layer", "Encountered opacity value not between 0-1. Clamping this to fit into that range");
		}
		value = std::clamp<float>(value, 0.0f, 1.0f);

		m_Opacity = static_cast<uint8_t>(value * 255.0f); 
	}

	/// The layers' width from 0 - 300,000
	virtual uint32_t width() const noexcept { return m_Width; }
	/// The layers' width from 0 - 300,000
	virtual void width(uint32_t layer_width)
	{  
		if (layer_width > static_cast<uint32_t>(300000))
		{
			PSAPI_LOG_ERROR("Layer", "Unable to set width to %u as the maximum layer size in photoshop is 300,000 for PSB", layer_width);
		}
		m_Width = layer_width;
	}

	/// The layers' height from 0 - 300,000
	virtual uint32_t height() const noexcept { return m_Height; }
	/// The layers' height from 0 - 300,000
	virtual void height(uint32_t layer_height)
	{
		if (layer_height > static_cast<uint32_t>(300000))
		{
			PSAPI_LOG_ERROR("Layer", "Unable to set height to %u as the maximum layer size in photoshop is 300,000 for PSB", layer_height);
		}
		m_Height = layer_height;
	}
	
	/// The layers' x center coordinate
	/// 
	/// I.e. if the layer has the bounds { 200, 200 } - { 1000, 1000 } The center
	/// would be at { 600, 600 }
	virtual float center_x() const noexcept { return m_CenterX; }
	virtual void center_x(float x_coord) noexcept { m_CenterX = x_coord; }

	/// The layers' y center coordinate
	/// 
	/// I.e. if the layer has the bounds { 200, 200 } - { 1000, 1000 } The center
	/// would be at { 600, 600 }
	virtual float center_y() const noexcept { return m_CenterY; }
	virtual void center_y(float y_coord) noexcept { m_CenterY = y_coord; }

	/// Convenience function for accessing the top left x coordinate of a layer
	float top_left_x() const noexcept { return m_CenterX - static_cast<float>(m_Width) / 2; }

	/// Convenience function for accessing the top left y coordinate of a layer
	float top_left_y() const noexcept { return m_CenterY - static_cast<float>(m_Height) / 2; }

	/// The color mode with which the file was created, only stored to
	/// allow better detection during channel access for e.g. image layers
	Enum::ColorMode color_mode() const noexcept { return m_ColorMode; }

	/// Set the write compression for all channels.
	///
	/// This has no effect on the in-memory compression of these channels but only on write.
	/// Setting this therefore has a near-zero runtime cost.
	/// 
	/// \param _compcode The new compression setting.
	virtual void set_write_compression(Enum::Compression _compcode)
	{
		MaskMixin<T>::set_mask_compression(_compcode);
	}

	Layer() : m_LayerName(""), m_BlendMode(Enum::BlendMode::Normal), m_IsVisible(true), m_Opacity(255), m_Width(0u), m_Height(0u), m_CenterX(0u), m_CenterY(0u) {};

	/// \brief Initialize a Layer instance from the internal Photoshop File Format structures.
	///
	/// This is part of the internal API and as a user you will likely never have to use 
	/// this function
	/// 
	/// This constructor is responsible for creating a Layer object based on the information
	/// stored in the provided Photoshop File Format structures. It extracts relevant data
	/// from the LayerRecord, ChannelImageData, and FileHeader to set up the Layer.
	///
	/// \param layerRecord The LayerRecord containing information about the layer.
	/// \param channelImageData The ChannelImageData holding the image data.
	/// \param header The FileHeader providing overall file information.
	Layer(const LayerRecord& layerRecord, ChannelImageData& channelImageData, const FileHeader& header)
	{
		m_ColorMode = header.m_ColorMode;
		m_LayerName = layerRecord.m_LayerName.getString();
		// To parse the blend mode we must actually check for the presence of the sectionDivider blendmode as this overrides the layerRecord
		// blendmode if it is present
		if (!layerRecord.m_AdditionalLayerInfo.has_value())
		{
			// Short circuit if no additional layer info is present
			m_BlendMode = layerRecord.m_BlendMode;
		}
		else
		{
			auto& additionalLayerInfo = layerRecord.m_AdditionalLayerInfo.value();
			auto sectionDivider = additionalLayerInfo.getTaggedBlock<LrSectionTaggedBlock>(Enum::TaggedBlockKey::lrSectionDivider);
			if (sectionDivider.has_value() && sectionDivider.value()->m_BlendMode.has_value())
			{
				m_BlendMode = sectionDivider.value()->m_BlendMode.value();
			}
			else
			{
				m_BlendMode = layerRecord.m_BlendMode;
			}

			// Parse the layer protection settings
			auto protectionSettings = additionalLayerInfo.getTaggedBlock<ProtectedSettingTaggedBlock>(Enum::TaggedBlockKey::lrProtectedSetting);
			if (protectionSettings)
			{
				m_IsLocked = protectionSettings.value()->m_IsLocked;
			}
			else
			{
				m_IsLocked = false;
			}
		}
		// For now we only parse visibility from the bitflags but this could be expanded to parse other information as well.
		m_IsVisible = !layerRecord.m_BitFlags.m_isHidden;
		m_IsClippingMask = static_cast<bool>(layerRecord.m_Clipping);
		if (m_IsLocked && !layerRecord.m_BitFlags.m_isTransparencyProtected)
		{
			PSAPI_LOG_WARNING("Layer", "Mismatch in parsing of protected layer settings detected. Expected both the layer to be locked and the transparency to be locked");
		}
		m_Opacity = layerRecord.m_Opacity;

		// Generate our coordinates from the extents
		Geometry::BoundingBox<float> bbox(
			Geometry::Point2D<float>(static_cast<float>(layerRecord.m_Left), static_cast<float>(layerRecord.m_Top)), 
			Geometry::Point2D<float>(static_cast<float>(layerRecord.m_Right), static_cast<float>(layerRecord.m_Bottom)));
		m_Width = static_cast<int32_t>(bbox.width());
		m_Height = static_cast<int32_t>(bbox.height());
		m_CenterX = bbox.center().x;
		m_CenterY = bbox.center().y;

		// Move the layer mask into our layerMask struct, for now this only does pixel masks
		for (int i = 0; i < layerRecord.m_ChannelCount; ++i)
		{
			auto& channelInfo = layerRecord.m_ChannelInformation[i];
			if (channelInfo.m_ChannelID == MaskMixin<T>::s_mask_index)
			{
				// Move the compressed image data into our LayerMask struct
				auto channelPtr = channelImageData.extractImagePtr(channelInfo.m_ChannelID);
				if (channelPtr)
				{
					MaskMixin<T>::m_MaskData = std::move(channelPtr);
				}
				else
				{
					PSAPI_LOG_ERROR("Layer", "Unable to extract mask channel for layer '%s'", m_LayerName.c_str());
				}

				// If no mask parameters are present we just use sensible defaults and skip
				if (!layerRecord.m_LayerMaskData.has_value())
				{
					continue;
				}
				auto& maskParams = layerRecord.m_LayerMaskData.value();

				// Read the mask parameters
				if (!maskParams.m_LayerMask.has_value())
				{
					continue;
				}
				auto& layerMaskParams = maskParams.m_LayerMask.value();

				MaskMixin<T>::mask_disabled(layerMaskParams.m_Disabled);
				MaskMixin<T>::mask_relative_to_layer(layerMaskParams.m_PositionRelativeToLayer);
				MaskMixin<T>::mask_default_color(layerMaskParams.m_DefaultColor);
				MaskMixin<T>::mask_density(layerMaskParams.m_UserMaskDensity);
				MaskMixin<T>::mask_feather(layerMaskParams.m_UserMaskFeather);

				// Forward the masks height as the layers width and height as photoshop does 
				// not store this explicitly when e.g. dealing with group layers.
				if (m_Width == 0 && m_Height == 0)
				{
					Geometry::BoundingBox<float> mask_bbox(
						Geometry::Point2D<float>(static_cast<float>(layerMaskParams.m_Left), static_cast<float>(layerMaskParams.m_Top)),
						Geometry::Point2D<float>(static_cast<float>(layerMaskParams.m_Right), static_cast<float>(layerMaskParams.m_Bottom)));
					m_Width = static_cast<int32_t>(mask_bbox.width());
					m_Height = static_cast<int32_t>(mask_bbox.height());
					m_CenterX = mask_bbox.center().x;
					m_CenterY = mask_bbox.center().y;					
				}
			}
		}

		if (layerRecord.m_AdditionalLayerInfo.has_value())
		{
			// Get the reference point (if it is there)
			auto& additionalLayerInfo = layerRecord.m_AdditionalLayerInfo.value();
			auto reference_point = additionalLayerInfo.get_tagged_block<ReferencePointTaggedBlock>();
			if (reference_point)
			{
				m_ReferencePointX.emplace(reference_point->m_ReferenceX);
				m_ReferencePointY.emplace(reference_point->m_ReferenceY);
			}
			
			// Get the unicode layer name (if it is there) and override the pascal string name
			auto unicode_name = additionalLayerInfo.get_tagged_block<UnicodeLayerNameTaggedBlock>();
			if (unicode_name)
			{
				m_LayerName = unicode_name->m_Name.string();
			}
		}
	}

	virtual ~Layer() = default;

	/// \brief Function for creating a PhotoshopFile compatible types from the layer.
	///
	/// This is part of the internal API and as a user you will likely never have to use 
	/// this function.
	/// 
	/// In the future, the intention is to make this a pure virtual function. However, due to
	/// the presence of multiple miscellaneous layers not yet implemented for the initial release,
	/// this function is provided. It generates a tuple containing LayerRecord and ChannelImageData
	/// based on the specified ColorMode, and using the provided FileHeader.
	///
	/// \return A tuple containing LayerRecord and ChannelImageData representing the layer in the PhotoshopFile.
	virtual std::tuple<LayerRecord, ChannelImageData> to_photoshop()
	{
		std::vector<LayerRecords::ChannelInformation> channelInfo{};	// Just have this be empty
		ChannelImageData channelData{};

		ChannelExtents extents = generate_extents(ChannelCoordinates(m_Width, m_Height, m_CenterX, m_CenterY));

		auto blockVec = this->generate_tagged_blocks();
		std::optional<AdditionalLayerInfo> taggedBlocks = std::nullopt;
		if (blockVec.size() > 0)
		{
			TaggedBlockStorage blockStorage = { blockVec };
			taggedBlocks.emplace(blockStorage);
		}

		LayerRecord lrRecord(
			PascalString(m_LayerName, 4u),	// Photoshop does sometimes explicitly write out the name such as '</Group 1>' to indicate what it belongs to 
			extents.top,
			extents.left,
			extents.bottom,
			extents.right,
			0u,		// Number of channels, photoshop does appear to actually write out all the channels with 0 length, we will see later if that is a requirement
			channelInfo,
			m_BlendMode,
			m_Opacity,
			static_cast<uint8_t>(m_IsClippingMask),
			LayerRecords::BitFlags(m_IsLocked, !m_IsVisible, false),
			std::nullopt,	// LayerMaskData
			Layer<T>::generate_blending_ranges(),	// Generate some defaults
			std::move(taggedBlocks)		// Additional layer information
		);

		return std::make_tuple(std::move(lrRecord), std::move(channelData));
	}

protected:

	std::string m_LayerName;

	Enum::BlendMode m_BlendMode = Enum::BlendMode::Normal;

	/// Marks whether or not the layer is visible or not
	bool m_IsVisible{};

	/// Whether the layer is locked inside of photoshop
	bool m_IsLocked = false;

	/// Whether the layer is a clipping mask to the layer below.
	bool m_IsClippingMask = false;

	/// 0 - 255 despite the appearance being 0-100 in photoshop
	uint8_t m_Opacity{};

	uint32_t m_Width{};

	uint32_t m_Height{};

	float m_CenterX{};

	float m_CenterY{};

	Enum::ColorMode m_ColorMode = Enum::ColorMode::RGB;

	/// Parse the layer mask passed as part of the parameters into m_LayerMask
	void parse_mask(Params& parameters)
	{
		if (parameters.mask)
		{
			auto mask_data = std::make_unique<ImageChannel>(
				parameters.compression,
				parameters.mask.value(),
				MaskMixin<T>::s_mask_index,
				parameters.width,
				parameters.height,
				static_cast<float>(parameters.center_x),
				static_cast<float>(parameters.center_y)
			);
			MaskMixin<T>::m_MaskData = std::move(mask_data);
		}
	}

	/// \brief Generate the layer name as a Pascal string.
	///
	/// \return A PascalString representing the layer name.
	PascalString generate_name()
	{
		return PascalString(m_LayerName, 4u);
	}

	/// \brief Generate the tagged blocks necessary for writing the layer
	virtual std::vector<std::shared_ptr<TaggedBlock>> generate_tagged_blocks()
	{
		std::vector<std::shared_ptr<TaggedBlock>> blockVec;
		// Generate our reference point tagged block
		if (m_ReferencePointX.has_value() && m_ReferencePointY.has_value())
		{
			auto referencePointPtr = std::make_shared<ReferencePointTaggedBlock>(m_ReferencePointX.value(), m_ReferencePointY.value());
			blockVec.push_back(referencePointPtr);
		}

		// Generate our unicode layer name block, we always include this as its size is trivial and this avoids 
		// any issues with names being truncated
		auto unicodeNamePtr = std::make_shared<UnicodeLayerNameTaggedBlock>(m_LayerName, static_cast<uint8_t>(4u));
		blockVec.push_back(unicodeNamePtr);

		// Generate our LockedSettings Tagged block
		auto protectionSettingsPtr = std::make_shared<ProtectedSettingTaggedBlock>(m_IsLocked);
		blockVec.push_back(protectionSettingsPtr);

		return blockVec;
	}

	/// \brief Generate the layer blending ranges (which for now are just the defaults).
	///
	/// \return A LayerBlendingRanges object representing the layer blending ranges.
	LayerRecords::LayerBlendingRanges generate_blending_ranges()
	{
		LayerRecords::LayerBlendingRanges blendingRanges{};
		return blendingRanges;
	}

	/// \brief Generate Default zero-length channels for the given colormode.
	/// 
	/// This should only be used on layers that do not already contain imagedata such as a GroupLayer<T> or a SectionDividerLayer<T> as some applications
	/// such as krita require zero-length layers to be present
	/// 
	/// \param channelInfoVec The channel infos to append to, will only add channels if the default keys dont already exist
	/// \param channelDataVec The channel data to append to, will only add channels if the default keys dont already exist
	/// \param colormode	  The colormode associated with the layer, dictates how many layers are actually written out
	void generate_empty_channels(std::vector<LayerRecords::ChannelInformation>& channelInfoVec, std::vector<std::unique_ptr<ImageChannel>>& channelDataVec, const Enum::ColorMode& colormode)
	{
		auto processChannel = [&](Enum::ChannelIDInfo channelid, int16_t i)
			{
				LayerRecords::ChannelInformation channelinfo = { .m_ChannelID = channelid, .m_Size = 0u };
				// Skip existing channels
				if (std::find(channelInfoVec.begin(), channelInfoVec.end(), channelinfo) != channelInfoVec.end())
				{
					PSAPI_LOG_DEBUG("Layer", "Skipped generation of default channel with ID: %d as it was already present on the data", static_cast<int>(i));
					return;
				}

				std::vector<T> empty(0);
				auto channel = std::make_unique<ImageChannel>(Enum::Compression::Raw, empty, channelid, 0u, 0u, 0.0f, 0.0f);
				channelInfoVec.push_back(channelinfo);
				channelDataVec.push_back(std::move(channel));
			};

		if (colormode == Enum::ColorMode::RGB)
		{
			// Fill channels {-1, 0, 1, 2}
			for (int16_t i = -1; i <= 2; ++i)
			{
				auto channelid = Enum::Impl::rgbIntToChannelID(i);
				processChannel(channelid, i);
			}
		}
		else if (colormode == Enum::ColorMode::CMYK)
		{
			// Fill channels {-1, 0, 1, 2, 3}
			for (int16_t i = -1; i <= 3; ++i)
			{
				auto channelid = Enum::Impl::cmykIntToChannelID(i);
				processChannel(channelid, i);
			}
		}
		else if (colormode == Enum::ColorMode::Grayscale)
		{
			// Fill channels {-1, 0}
			for (int16_t i = -1; i <= 0; ++i)
			{
				auto channelid = Enum::Impl::grayscaleIntToChannelID(i);
				processChannel(channelid, i);
			}
		}
	}


	/// Optional argument which specifies in global coordinates where the top left of the layer is to e.g. flip or rotate a layer
	/// currently this is only used for roundtripping, therefore optional. This value must be within the layers bounding box (or no
	/// more than .5 away since it is a double)
	std::optional<double> m_ReferencePointX = std::nullopt;
	/// Optional argument which specifies in global coordinates where the top left of the layer is to e.g. flip or rotate a layer
	/// currently this is only used for roundtripping, therefore optional. This value must be within the layers bounding box (or no
	/// more than .5 away since it is a double)
	std::optional<double> m_ReferencePointY = std::nullopt;
};


extern template struct Layer<bpp8_t>;
extern template struct Layer<bpp16_t>;
extern template struct Layer<bpp32_t>;

PSAPI_NAMESPACE_END