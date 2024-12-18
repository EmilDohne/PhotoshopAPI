#pragma once

#include "Macros.h"
#include "Util/Enum.h"
#include "Core/Struct/ImageChannel.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "PhotoshopFile/AdditionalLayerInfo.h"
#include "Core/Struct/TaggedBlock.h"

#include <vector>
#include <optional>
#include <string>
#include <memory>

PSAPI_NAMESPACE_BEGIN

/// Structure describing a pixel based mask layer with the image data stored on the `data` member. This will usually be used in the context
/// of a `Layer<T>` which has additional functionality for getting the image data. If however you wish to modify information such as 
/// the feather, density or disabled state of the mask this struct is what you will have to modify.
struct LayerMask
{
	/// The data stored as an ImageChannel struct. If you accessed this mask in the context of a layer you should instead use the 
	/// Layer::get_mask_data() and Layer::set_mask_data() functions.
	std::unique_ptr<ImageChannel> data;

	/// For internal parsing only.
	/// Whether the mask channels' position is relative to the layer. This only matters for how the internal coordinates are parsed
	bool relative_to_layer = false;	

	/// Whether the mask is disabled
	bool disabled = false;

	/// The masks' default color. This defines the background color if you will. Photoshop internally only stores a
	/// tight bounding box of the mask data that is relevant (i.e. not filled with one color). If you wish to compose
	/// the mask channel for the whole layer this value must be taken into account. The PhotoshopAPI on the other hand
	/// will store the mask channel in its entirety as compression will take care of pretty much all of the empty space.
	/// 
	/// Therefore, if you read a photoshop file created by photoshop you must take this value into account. If instead
	/// you create a Photoshop file via the API this value can be safely ignored
	uint8_t default_color = 255u;

	/// The mask's density parameter, if this is not present this is just the default of 255 (100%).
	std::optional<uint8_t> density;

	/// The mask's feather parameter, if this is not present this is just the default of no feather
	std::optional<float64_t> feather;

	LayerMask() = default;
};

/// Base Struct for Layers of all types (Group, Image, [Adjustment], etc.) which includes the minimum to parse a generic layer type
template <typename T>
struct Layer
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
		/// The X Center coordinate, 0 indicates that the image is centered around the document, a negative value moves the layer to the left
		int32_t center_x = 0;
		/// The Y Center coordinate, 0 indicates that the image is centered around the document, a negative value moves the layer to the top
		int32_t center_y = 0;
		/// The width of the layer, this value must be passed explicitly as we do not deduce this from the Image Data itself
		uint32_t width = 0u;
		/// The height of the layer, this value must be passed explicitly as we do not deduce this from the Image Data itself
		uint32_t height = 0u;
		/// The Layer opacity, the value displayed by Photoshop will be this value / 2.55 so 255 corresponds to 100% 
		/// while 128 would correspond to ~50%
		uint8_t opacity = 255u;
		// The compression codec of the layer, it is perfectly valid for each layer to be compressed differently
		Enum::Compression compression = Enum::Compression::ZipPrediction; 
		// The Layers color mode, currently only RGB is supported
		Enum::ColorMode colormode = Enum::ColorMode::RGB;
		// Whether the layer is visible
		bool visible = true;
		// Whether the layer is locked
		bool locked = false;
	};

	/// The layers' name. Stored as a utf-8 string
	const std::string& name() const noexcept { return m_LayerName; }

	/// The layers' name. Stored as a utf-8 string
	std::string& name() noexcept { return m_LayerName; }

	/// The layers' name. Stored as a utf-8 string
	void name(const std::string& layer_name) noexcept { m_LayerName = layer_name; }

	/// An optional mask component.
	/// At this time only raster (pixel) masks are supported with support for vector masks following.
	std::optional<LayerMask>& mask() { return m_LayerMask; }

	/// An optional mask component.
	/// At this time only raster (pixel) masks are supported with support for vector masks following.
	const std::optional<LayerMask>& mask() const { return m_LayerMask; }

	/// Extract the mask data as a vector
	///
	/// If `copy` is set to true we copy out the data while if it set to false
	/// we extract and invalidate the mask essentially removing the mask from the layer.
	/// If no mask is present we raise a warning and return an empty vector
	/// 
	/// \param copy Whether to copy the data, defaults to `true`
	/// 
	/// \returns The mask data or an empty vector
	std::vector<T> get_mask_data(bool copy = true)
	{
		if (m_LayerMask.has_value())
		{
			if (copy)
			{
				return m_LayerMask.value().data->getData<T>();
			}
			else
			{
				auto data = m_LayerMask.value().data->extractData<T>();
				m_LayerMask = std::nullopt;
				return std::move(data);
			}
		}
		PSAPI_LOG_WARNING("Layer", "Layer doesnt have a mask channel, returning an empty vector<T>");
		return std::vector<T>();
	}

	/// Set the layers' mask
	///
	/// \param data The data to set the mask channel to, must have the dimensions 
	///				width * height. If that isn't the case you must first call 
	///				`layer->width()` and `layer->height()` to update this.
	void set_mask_data(std::span<const T> data)
	{
		LayerMask newMask{};
		if (data.size() != static_cast<size_t>(m_Width) * m_Height)
		{
			PSAPI_LOG_ERROR("Layer", "Unable to set mask channel as new masks' size is not the same as the layers' width and height");
		}
		auto idinfo = Enum::toChannelIDInfo(Enum::ChannelID::UserSuppliedLayerMask, m_ColorMode);
		newMask.data = std::make_unique<ImageChannel>(Enum::Compression::ZipPrediction, data, idinfo, m_Width, m_Height, m_CenterX, m_CenterY);
		m_LayerMask.emplace(std::move(newMask));
	}

	/// Check whether a mask channel exists on the layer instance
	bool has_mask() { return m_LayerMask.has_value(); }

	
	/// The blendmode of the layer, the `Passthrough` blendmode is only valid for groups
	Enum::BlendMode& blendmode() noexcept { return m_BlendMode; }
	/// The blendmode of the layer, the `Passthrough` blendmode is only valid for groups
	Enum::BlendMode blendmode() const noexcept { return m_BlendMode; }
	/// The blendmode of the layer, the `Passthrough` blendmode is only valid for groups
	void blendmode(Enum::BlendMode blend_mode) noexcept { return m_BlendMode = blend_mode; }

	
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

	/// The layers' opacity. 
	/// 
	/// In photoshop this is stored as a `uint8_t` from 0-255 but access and write is 
	/// in terms of a float for better consistency.
	float opacity() const noexcept { return static_cast<float>(m_Opacity) / 255; }
	/// The layers' opacity. 
	/// 
	/// In photoshop this is stored as a `uint8_t` from 0-255 but access and write is 
	/// in terms of a float for better consistency.
	void opacity(float value) noexcept { m_Opacity = static_cast<uint8_t>(value * 255.0f); }

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
	
	/// The layers' x coordinate
	/// 
	/// These are represented as the layer's center.
	/// I.e. if the layer has the bounds { 200, 200 } - { 1000, 1000 } The center
	/// would be at { 600, 600 }
	virtual float center_x() const noexcept { return m_CenterX; }
	virtual void center_x(float x_coord) noexcept { m_CenterX = x_coord; }

	/// The layers' y coordinate
	/// 
	/// These are represented as the layer's center.
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

	virtual ~Layer() = default;

protected:

	std::string m_LayerName;

	/// A pixel layer mask
	std::optional<LayerMask> m_LayerMask;

	Enum::BlendMode m_BlendMode = Enum::BlendMode::Normal;

	/// Marks whether or not the layer is visible or not
	bool m_IsVisible{};

	/// Whether the layer is locked inside of photoshop
	bool m_IsLocked = false;

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
			LayerMask mask{};
			Enum::ChannelIDInfo info{ .id = Enum::ChannelID::UserSuppliedLayerMask, .index = -2 };
			mask.data = std::make_unique<ImageChannel>(
				parameters.compression,
				parameters.mask.value(),
				info,
				parameters.width,
				parameters.height,
				static_cast<float>(parameters.center_x),
				static_cast<float>(parameters.center_y)
			);
			Layer<T>::m_LayerMask = std::move(mask);
		}
	}

	/// \brief Generates the LayerMaskData struct from the layer mask (if provided).
	/// 
	/// Part of the internal API, not expected to be used by users.
	///
	/// \return An optional containing LayerMaskData if a layer mask is present; otherwise, std::nullopt.
	std::optional<LayerRecords::LayerMaskData> generate_mask(const FileHeader& header)
	{
		auto lrMaskData = LayerRecords::LayerMaskData();

		// We dont have support for vector masks so far so we do not consider them
		lrMaskData.m_VectorMask = std::nullopt;
		if (!m_LayerMask.has_value())
		{
			lrMaskData.m_LayerMask = std::nullopt;
		}
		else
		{
			LayerRecords::LayerMask lrMask = LayerRecords::LayerMask{};

			float centerX = m_LayerMask.value().data->getCenterX();
			float centerY = m_LayerMask.value().data->getCenterY();
			int32_t width = m_LayerMask.value().data->getWidth();
			int32_t height = m_LayerMask.value().data->getHeight();
			ChannelExtents extents = generate_extents(ChannelCoordinates(width, height, centerX, centerY), header);
			lrMaskData.addSize(16u);

			// Default color
			lrMaskData.addSize(1u);

			// This is the size for the mask bitflags
			lrMaskData.addSize(1u);
			// This is the size for the mask parameters
			lrMaskData.addSize(1u);
			bool hasMaskDensity = m_LayerMask.value().density.has_value();
			uint8_t maskDensity = 0u;
			if (hasMaskDensity)
			{
				maskDensity = m_LayerMask.value().density.value();
				lrMaskData.addSize(1u);
			}

			bool hasMaskFeather = m_LayerMask.value().feather.has_value();
			float64_t maskFeather = 0.0f;
			if (hasMaskFeather)
			{
				maskFeather = m_LayerMask.value().feather.value();
				lrMaskData.addSize(8u);

			}

			lrMask.m_Top = extents.top;
			lrMask.m_Left = extents.left;
			lrMask.m_Bottom = extents.bottom;
			lrMask.m_Right = extents.right;
			lrMask.m_DefaultColor = m_LayerMask.value().default_color;
			lrMask.m_Disabled = m_LayerMask.value().disabled;
			lrMask.m_PositionRelativeToLayer = m_LayerMask.value().relative_to_layer;
			lrMask.m_HasMaskParams = hasMaskDensity || hasMaskFeather;
			lrMask.m_HasUserMaskDensity = hasMaskDensity;
			lrMask.m_HasUserMaskFeather = hasMaskFeather;
			if (hasMaskDensity)
			{
				lrMask.m_UserMaskDensity.emplace(maskDensity);
			}
			if (hasMaskFeather)
			{
				lrMask.m_UserMaskFeather.emplace(maskFeather);
			}

			lrMaskData.m_LayerMask.emplace(lrMask);
		}
		

		std::optional<LayerRecords::LayerMaskData> lrMaskDataOpt;
		lrMaskDataOpt.emplace(lrMaskData);
		if (!lrMaskData.m_LayerMask.has_value() && !lrMaskData.m_VectorMask.has_value())
		{
			return std::nullopt;
		}
		return lrMaskDataOpt;
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

	/// \brief Extract the layer mask into a tuple of channel information and image data
	///
	/// \return An optional containing a tuple of ChannelInformation and a unique_ptr to BaseImageChannel.
	std::optional<std::tuple<LayerRecords::ChannelInformation, std::unique_ptr<ImageChannel>>> extract_mask()
	{
		if (!m_LayerMask.has_value())
		{
			return std::nullopt;
		}
		std::unique_ptr<ImageChannel> maskImgChannel = std::move(m_LayerMask.value().data);
		Enum::ChannelIDInfo maskIdInfo{ Enum::ChannelID::UserSuppliedLayerMask, -2 };
		LayerRecords::ChannelInformation channelInfo{ maskIdInfo, maskImgChannel->m_OrigByteSize };
		std::tuple<LayerRecords::ChannelInformation, std::unique_ptr<ImageChannel>> data = std::make_tuple(channelInfo, std::move(maskImgChannel));
		return std::optional(std::move(data));
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

public:

	Layer() : m_LayerName(""), m_LayerMask({}), m_BlendMode(Enum::BlendMode::Normal), m_IsVisible(true), m_Opacity(255), m_Width(0u), m_Height(0u), m_CenterX(0u), m_CenterY(0u) {};

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
		if (m_IsLocked && !layerRecord.m_BitFlags.m_isTransparencyProtected)
		{
			PSAPI_LOG_WARNING("Layer", "Mismatch in parsing of protected layer settings detected. Expected both the layer to be locked and the transparency to be locked");
		}
		m_Opacity = layerRecord.m_Opacity;

		// Generate our coordinates from the extents
		ChannelExtents extents{ layerRecord.m_Top, layerRecord.m_Left, layerRecord.m_Bottom, layerRecord.m_Right };
		ChannelCoordinates coordinates = generateChannelCoordinates(extents, header);
		m_Width = coordinates.width;
		m_Height = coordinates.height;
		m_CenterX = coordinates.centerX;
		m_CenterY = coordinates.centerY;

		// Move the layer mask into our layerMask struct, for now this only does pixel masks
		for (int i = 0; i < layerRecord.m_ChannelCount; ++i)
		{
			auto& channelInfo = layerRecord.m_ChannelInformation[i];
			if (channelInfo.m_ChannelID.id == Enum::ChannelID::UserSuppliedLayerMask)
			{
				// Move the compressed image data into our LayerMask struct
				LayerMask lrMask{};
				auto channelPtr = channelImageData.extractImagePtr(channelInfo.m_ChannelID);
				if (channelPtr)
				{
					lrMask.data = std::move(channelPtr);
				}
				else
				{
					PSAPI_LOG_ERROR("Layer", "Unable to extract mask channel for layer '%s'", m_LayerName.c_str());
				}
				channelPtr = nullptr;

				// If no mask parameters are present we just use sensible defaults and skip
				if (!layerRecord.m_LayerMaskData.has_value()) continue;
				auto& maskParams = layerRecord.m_LayerMaskData.value();

				// Read the mask parameters
				if (!maskParams.m_LayerMask.has_value()) continue;
				auto& layerMaskParams = maskParams.m_LayerMask.value();

				lrMask.disabled = layerMaskParams.m_Disabled;
				lrMask.relative_to_layer = layerMaskParams.m_PositionRelativeToLayer;
				lrMask.default_color = layerMaskParams.m_DefaultColor;
				lrMask.density = layerMaskParams.m_UserMaskDensity;
				lrMask.feather = layerMaskParams.m_UserMaskFeather;


				// Forward the masks height as the layers width and height
				// As photoshop does not store this explicitly
				if (m_Width == 0 && m_Height == 0)
				{
					ChannelExtents extents{ layerMaskParams.m_Top, layerMaskParams.m_Left, layerMaskParams.m_Bottom, layerMaskParams.m_Right };
					ChannelCoordinates coordinates = generateChannelCoordinates(extents, header);
					m_Width = coordinates.width;
					m_Height = coordinates.height;
					m_CenterX = coordinates.centerX;
					m_CenterY = coordinates.centerY;
				}

				// Set the layer mask by moving our temporary struct
				m_LayerMask = std::optional(std::move(lrMask));
			}
		}

		// Get the reference point (if it is there)
		if (layerRecord.m_AdditionalLayerInfo.has_value())
		{
			auto& additionalLayerInfo = layerRecord.m_AdditionalLayerInfo.value();
			auto referencePoint = additionalLayerInfo.getTaggedBlock<ReferencePointTaggedBlock>(Enum::TaggedBlockKey::lrReferencePoint);
			if (referencePoint.has_value())
			{
				m_ReferencePointX.emplace(referencePoint.value()->m_ReferenceX);
				m_ReferencePointY.emplace(referencePoint.value()->m_ReferenceY);
			}
		}
		// Get the unicode layer name (if it is there) and override the pascal string name
		if (layerRecord.m_AdditionalLayerInfo.has_value())
		{
			auto& additionalLayerInfo = layerRecord.m_AdditionalLayerInfo.value();
			auto unicodeName = additionalLayerInfo.getTaggedBlock<UnicodeLayerNameTaggedBlock>(Enum::TaggedBlockKey::lrUnicodeName);
			if (unicodeName.has_value())
			{
				m_LayerName = unicodeName.value()->m_Name.getString();
			}
		}
	}

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
	/// \param colorMode The desired ColorMode for the PhotoshopFile.
	/// \param header The FileHeader providing overall file information.
	/// \return A tuple containing LayerRecord and ChannelImageData representing the layer in the PhotoshopFile.
	virtual std::tuple<LayerRecord, ChannelImageData> to_photoshop([[maybe_unused]] const Enum::ColorMode colorMode, const FileHeader& header)
	{
		std::vector<LayerRecords::ChannelInformation> channelInfo{};	// Just have this be empty
		ChannelImageData channelData{};

		ChannelExtents extents = generate_extents(ChannelCoordinates(m_Width, m_Height, m_CenterX, m_CenterY), header);

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
			0u,		// Clipping
			LayerRecords::BitFlags(m_IsLocked, !m_IsVisible, false),
			std::nullopt,	// LayerMaskData
			Layer<T>::generate_blending_ranges(),	// Generate some defaults
			std::move(taggedBlocks)		// Additional layer information
		);

		return std::make_tuple(std::move(lrRecord), std::move(channelData));
	}


	/// Changes the compression mode of all channels in this layer to the given compression mode
	virtual void set_compression(const Enum::Compression compCode)
	{
		if (m_LayerMask)
		{
			m_LayerMask.value().data->m_Compression = compCode;
		}
	}

protected:

	/// Optional argument which specifies in global coordinates where the top left of the layer is to e.g. flip or rotate a layer
	/// currently this is only used for roundtripping, therefore optional. This value must be within the layers bounding box (or no
	/// more than .5 away since it is a double)
	std::optional<double> m_ReferencePointX = std::nullopt;
	/// Optional argument which specifies in global coordinates where the top left of the layer is to e.g. flip or rotate a layer
	/// currently this is only used for roundtripping, therefore optional. This value must be within the layers bounding box (or no
	/// more than .5 away since it is a double)
	std::optional<double> m_ReferencePointY = std::nullopt;
};


PSAPI_NAMESPACE_END