#include "Layer.h"

#include "Macros.h"
#include "Enum.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "PhotoshopFile/AdditionalLayerInfo.h"
#include "Struct/TaggedBlock.h"

#include <vector>
#include <optional>
#include <string>
#include <memory>

PSAPI_NAMESPACE_BEGIN


// Instantiate the template types for Layer
template struct Layer<uint8_t>;
template struct Layer<uint16_t>;
template struct Layer<float32_t>;


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
Layer<T>::Layer(const LayerRecord& layerRecord, ChannelImageData& channelImageData, const FileHeader& header)
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
			LayerMask<T> lrMask;
			auto channelPtr = channelImageData.extractImagePtr(channelInfo.m_ChannelID);
			ImageChannel<T>* imageChannelPtr = dynamic_cast<ImageChannel<T>*>(channelPtr.get());
			
			if (imageChannelPtr)
			{
				lrMask.maskData = std::move(*imageChannelPtr);
			}
			else
			{
				PSAPI_LOG_ERROR("Layer", "Unable to cast mask to ImageChannel");
			}
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
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::optional<LayerRecords::LayerMaskData> Layer<T>::generateMaskData(const FileHeader& header)
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

		float centerX = m_LayerMask.value().maskData.getCenterX();
		float centerY = m_LayerMask.value().maskData.getCenterY();
		int32_t width = m_LayerMask.value().maskData.getWidth();
		int32_t height = m_LayerMask.value().maskData.getHeight();
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


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
PascalString Layer<T>::generatePascalString()
{
	return PascalString(m_LayerName, 4u);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<std::shared_ptr<TaggedBlock>> Layer<T>::generateTaggedBlocks()
{
	std::vector<std::shared_ptr<TaggedBlock>> blockVec;
	// Generate our reference point tagged block
	if (m_ReferencePointX.has_value() && m_ReferencePointY.has_value())
	{
		auto referencePointPtr = std::make_shared<ReferencePointTaggedBlock>(m_ReferencePointX.value(), m_ReferencePointY.value());
		blockVec.push_back(referencePointPtr);
	}

	return blockVec;
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
LayerRecords::LayerBlendingRanges Layer<T>::generateBlendingRanges(const Enum::ColorMode colorMode)
{
	using Data = std::vector<std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>>;
	LayerRecords::LayerBlendingRanges blendingRanges{};
	return blendingRanges;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::optional<std::tuple<LayerRecords::ChannelInformation, std::unique_ptr<BaseImageChannel>>> Layer<T>::extractLayerMask(bool doCopy)
{
	if (!m_LayerMask.has_value())
	{
		return std::nullopt;
	}

	auto& maskImgChannel = m_LayerMask.value().maskData;
	Enum::ChannelIDInfo maskIdInfo{ Enum::ChannelID::UserSuppliedLayerMask, -2 };
	LayerRecords::ChannelInformation channelInfo{ maskIdInfo, maskImgChannel.m_OrigByteSize };

	// TODO this might not be doing much at all
	if (doCopy)
	{
		std::unique_ptr<ImageChannel<T>> imgData = std::make_unique<ImageChannel<T>>(maskImgChannel);
		std::tuple<LayerRecords::ChannelInformation, std::unique_ptr<ImageChannel<T>>> data = std::make_tuple(channelInfo, std::move(imgData));
		return std::optional(std::move(data));
	}
	else
	{
		std::unique_ptr<ImageChannel<T>> imgData = std::make_unique<ImageChannel<T>>(std::move(maskImgChannel));
		std::tuple<LayerRecords::ChannelInformation, std::unique_ptr<ImageChannel<T>>> data = std::make_tuple(channelInfo, std::move(imgData));
		return std::optional(std::move(data));
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::tuple<LayerRecord, ChannelImageData> Layer<T>::toPhotoshop(const Enum::ColorMode colorMode, const bool doCopy, const FileHeader& header)
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


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::vector<T> Layer<T>::getMaskData(const bool doCopy /*= true*/)
{
	if (m_LayerMask.has_value())
	{
		if (doCopy)
		{
			return std::move(m_LayerMask.value().maskData.getData());
		}
		else
		{
			return std::move(m_LayerMask.value().maskData.extractData());
		}
	}
	PSAPI_LOG_WARNING("Layer", "Layer doesnt have a mask channel, returning an empty vector<T>");
	return std::vector<T>();
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
void Layer<T>::setCompression(const Enum::Compression compCode)
{
	if (m_LayerMask.has_value())
	{
		m_LayerMask.value().maskData.m_Compression = compCode;
	}
}



PSAPI_NAMESPACE_END