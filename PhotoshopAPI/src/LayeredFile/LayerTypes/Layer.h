#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Core/Struct/ImageChannel.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "PhotoshopFile/AdditionalLayerInfo.h"
#include "Core/Struct/TaggedBlock.h"

#include <vector>
#include <optional>
#include <string>
#include <memory>

PSAPI_NAMESPACE_BEGIN

/// Structure describing a layer mask (pixel based) 
struct LayerMask
{
	std::unique_ptr<ImageChannel> maskData;
	bool isMaskRelativeToLayer = false;	/// This is primarily for roundtripping and the user shouldnt have to touch this
	bool isDisabled = false;
	uint8_t defaultColor = 255u;
	std::optional<uint8_t> maskDensity;
	std::optional<float64_t> maskFeather;

	LayerMask() = default;
};

/// Base Struct for Layers of all types (Group, Image, [Adjustment], etc.) which includes the minimum to parse a generic layer type
template <typename T>
struct Layer
{
	/// Layer Parameters for initialization of a generic layer type. It provides sensible defaults so only what is needed
	/// needs to be overridden
	struct Params
	{
		/// Optional Layer Mask parameter, if none is specified there is no mask. This image data must have the same size as 
		/// the layer itself
		std::optional<std::vector<T>> layerMask = std::nullopt;
		/// The Layer Name to give to the layer, has a maximum length of 255
		std::string layerName = "";
		/// The Layers Blend Mode, all available blend modes are valid except for 'Passthrough' on non-group layers
		Enum::BlendMode blendMode = Enum::BlendMode::Normal;
		/// The X Center coordinate, 0 indicates that the image is centered around the document, a negative value moves the layer to the left
		int32_t posX = 0;
		/// The Y Center coordinate, 0 indicates that the image is centered around the document, a negative value moves the layer to the top
		int32_t posY = 0;
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
		Enum::ColorMode colorMode = Enum::ColorMode::RGB;
	};

	std::string m_LayerName;

	/// A pixel layer mask
	std::optional<LayerMask> m_LayerMask;

	Enum::BlendMode m_BlendMode;

	/// Marks whether or not the layer is visible or not
	bool m_IsVisible; 

	/// 0 - 255 despite the appearance being 0-100 in photoshop
	uint8_t m_Opacity;	

	uint32_t m_Width;

	uint32_t m_Height;

	float m_CenterX;

	float m_CenterY;

protected:

	/// \brief Generates the LayerMaskData struct from the layer mask (if provided).
	///
	/// \return An optional containing LayerMaskData if a layer mask is present; otherwise, std::nullopt.
	std::optional<LayerRecords::LayerMaskData> generateMaskData(const FileHeader& header)
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

			float centerX = m_LayerMask.value().maskData->getCenterX();
			float centerY = m_LayerMask.value().maskData->getCenterY();
			int32_t width = m_LayerMask.value().maskData->getWidth();
			int32_t height = m_LayerMask.value().maskData->getHeight();
			ChannelExtents extents = generateChannelExtents(ChannelCoordinates(width, height, centerX, centerY), header);
			lrMaskData.m_Size += 16u;

			// Default color
			lrMaskData.m_Size += 1u;

			// This is the size for the mask bitflags
			lrMaskData.m_Size += 1u;
			// This is the size for the mask parameters
			lrMaskData.m_Size += 1u;
			bool hasMaskDensity = m_LayerMask.value().maskDensity.has_value();
			uint8_t maskDensity = 0u;
			if (hasMaskDensity)
			{
				maskDensity = m_LayerMask.value().maskDensity.value();
				lrMaskData.m_Size += 1u;
			}

			bool hasMaskFeather = m_LayerMask.value().maskFeather.has_value();
			float64_t maskFeather = 0.0f;
			if (hasMaskFeather)
			{
				maskFeather = m_LayerMask.value().maskFeather.value();
				lrMaskData.m_Size += 8u;

			}

			lrMask.m_Top = extents.top;
			lrMask.m_Left = extents.left;
			lrMask.m_Bottom = extents.bottom;
			lrMask.m_Right = extents.right;
			lrMask.m_DefaultColor = m_LayerMask.value().defaultColor;
			lrMask.m_Disabled = m_LayerMask.value().isDisabled;
			lrMask.m_PositionRelativeToLayer = m_LayerMask.value().isMaskRelativeToLayer;
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
	PascalString generatePascalString()
	{
		return PascalString(m_LayerName, 4u);
	}

	/// \brief Generate the tagged blocks necessary for writing the layer
	virtual std::vector<std::shared_ptr<TaggedBlock>> generateTaggedBlocks()
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
		auto unicodeNamePtr = std::make_shared<UnicodeLayerNameTaggedBlock>(m_LayerName, 4u);
		blockVec.push_back(unicodeNamePtr);

		return blockVec;
	}

	/// \brief Generate the layer blending ranges (which for now are just the defaults).
	///
	/// The blending ranges depend on the specified ColorMode. This function returns the default
	/// blending ranges for the given color mode.
	///
	/// \param colorMode The ColorMode to determine the blending ranges.
	/// \return A LayerBlendingRanges object representing the layer blending ranges.
	LayerRecords::LayerBlendingRanges generateBlendingRanges(const Enum::ColorMode colorMode)
	{
		using Data = std::vector<std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>>;
		LayerRecords::LayerBlendingRanges blendingRanges{};
		return blendingRanges;
	}

	/// \brief Extract the layer mask into a tuple of channel information and image data
	///
	/// \return An optional containing a tuple of ChannelInformation and a unique_ptr to BaseImageChannel.
	std::optional<std::tuple<LayerRecords::ChannelInformation, std::unique_ptr<ImageChannel>>> extractLayerMask()
	{
		if (!m_LayerMask.has_value())
		{
			return std::nullopt;
		}
		auto maskImgChannel = std::move(m_LayerMask.value().maskData);
		Enum::ChannelIDInfo maskIdInfo{ Enum::ChannelID::UserSuppliedLayerMask, -2 };
		LayerRecords::ChannelInformation channelInfo{ maskIdInfo, maskImgChannel->m_OrigByteSize };
		std::tuple<LayerRecords::ChannelInformation, std::unique_ptr<ImageChannel>> data = std::make_tuple(channelInfo, std::move(maskImgChannel));
		return std::optional(std::move(data));
	}

public:

	Layer() : m_LayerName(""), m_LayerMask({}), m_BlendMode(Enum::BlendMode::Normal), m_IsVisible(true), m_Opacity(255), m_Width(0u), m_Height(0u), m_CenterX(0u), m_CenterY(0u) {};

	/// \brief Initialize a Layer instance from the internal Photoshop File Format structures.
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
		m_LayerName = layerRecord.m_LayerName.getString();
		// To parse the blend mode we must actually check for the presence of the sectionDivider blendMode as this overrides the layerRecord
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
		}
		// For now we only parse visibility from the bitflags but this could be expanded to parse other information as well.
		m_IsVisible = !layerRecord.m_BitFlags.m_isHidden;
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
					lrMask.maskData = std::move(channelPtr);
				else
					PSAPI_LOG_ERROR("Layer", "Unable to extract mask channel for layer '%s'", m_LayerName.c_str());
				channelPtr = nullptr;

				// If no mask parameters are present we just use sensible defaults and skip
				if (!layerRecord.m_LayerMaskData.has_value()) continue;
				auto& maskParams = layerRecord.m_LayerMaskData.value();

				// Read the mask parameters
				if (!maskParams.m_LayerMask.has_value()) continue;
				auto& layerMaskParams = maskParams.m_LayerMask.value();

				lrMask.isDisabled = layerMaskParams.m_Disabled;
				lrMask.isMaskRelativeToLayer = layerMaskParams.m_PositionRelativeToLayer;
				lrMask.defaultColor = layerMaskParams.m_DefaultColor;
				lrMask.maskDensity = layerMaskParams.m_UserMaskDensity;
				lrMask.maskFeather = layerMaskParams.m_UserMaskFeather;

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

	/// \brief Function for creating a PhotoshopFile from the layer.
	///
	/// In the future, the intention is to make this a pure virtual function. However, due to
	/// the presence of multiple miscellaneous layers not yet implemented for the initial release,
	/// this function is provided. It generates a tuple containing LayerRecord and ChannelImageData
	/// based on the specified ColorMode, and using the provided FileHeader.
	///
	/// \param colorMode The desired ColorMode for the PhotoshopFile.
	/// \param header The FileHeader providing overall file information.
	/// \return A tuple containing LayerRecord and ChannelImageData representing the layer in the PhotoshopFile.
	virtual std::tuple<LayerRecord, ChannelImageData> toPhotoshop(Enum::ColorMode colorMode, const FileHeader& header)
	{
		std::vector<LayerRecords::ChannelInformation> channelInfo{};	// Just have this be empty
		ChannelImageData channelData{};

		ChannelExtents extents = generateChannelExtents(ChannelCoordinates(m_Width, m_Height, m_CenterX, m_CenterY), header);

		auto blockVec = this->generateTaggedBlocks();
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
			LayerRecords::BitFlags(false, !m_IsVisible, false),
			std::nullopt,	// LayerMaskData
			Layer<T>::generateBlendingRanges(colorMode),	// Generate some defaults
			std::move(taggedBlocks)		// Additional layer information
		);

		return std::make_tuple(std::move(lrRecord), std::move(channelData));
	}
	
	/// Extract the mask data as a vector, if doCopy is false the image data is freed and no longer usable
	std::vector<T> getMaskData(const bool doCopy = true)
	{
		if (m_LayerMask.has_value())
		{
			if (doCopy)
				return m_LayerMask.value().maskData->getData<T>();
			else
				return m_LayerMask.value().maskData->extractData<T>();
		}
		PSAPI_LOG_WARNING("Layer", "Layer doesnt have a mask channel, returning an empty vector<T>");
		return std::vector<T>();
	}

	/// Changes the compression mode of all channels in this layer to the given compression mode
	virtual void setCompression(const Enum::Compression compCode)
	{
		if (m_LayerMask.has_value())
		m_LayerMask.value().maskData->m_Compression = compCode;
	}

	virtual ~Layer() = default;

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