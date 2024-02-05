#include "ImageLayer.h"

#include "Layer.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"

#include <vector>
#include <unordered_map>
#include <optional>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>


PSAPI_NAMESPACE_BEGIN


// Instantiate the template types for ImageLayer
template struct ImageLayer<uint8_t>;
template struct ImageLayer<uint16_t>;
template struct ImageLayer<float32_t>;


namespace
{
	// Check that the map of channels has the required amount of keys. For example for an RGB image R, G and B must be present.
	// Returns false if a specific key is not found
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	bool checkChannelKeys(const std::unordered_map<Enum::ChannelIDInfo, ImageChannel<T>, Enum::ChannelIDInfoHasher>& data, const std::vector<Enum::ChannelIDInfo>& requiredKeys)
	{
		for (const auto& requiredKey : requiredKeys)
		{
			auto it = data.find(requiredKey);
			if (it == data.end())
			{
				return false;
			}
		}
		return true;
	}

}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::tuple<LayerRecord, ChannelImageData> ImageLayer<T>::toPhotoshop(const Enum::ColorMode colorMode, const bool doCopy, const FileHeader& header)
{
	PascalString lrName = Layer<T>::generatePascalString();
	auto extents = Layer<T>::generateExtents(header);
	int32_t top = std::get<0>(extents);
	int32_t left = std::get<1>(extents);
	int32_t bottom = std::get<2>(extents);
	int32_t right = std::get<3>(extents);
	uint16_t channelCount = m_ImageData.size() + static_cast<uint16_t>(Layer<T>::m_LayerMask.has_value());

	// Initialize the channel information as well as the channel image data, the size held in the channelInfo might change depending on
	// the compression mode chosen on export and must therefore be updated later.
	auto channelData = this->extractImageData(doCopy);
	auto& channelInfoVec = std::get<0>(channelData);
	ChannelImageData channelImgData = std::move(std::get<1>(channelData));

	uint8_t clipping = 0u;	// No clipping mask for now
	LayerRecords::BitFlags bitFlags(false, !Layer<T>::m_IsVisible, false);
	std::optional<LayerRecords::LayerMaskData> lrMaskData = Layer<T>::generateMaskData();
	LayerRecords::LayerBlendingRanges blendingRanges = Layer<T>::generateBlendingRanges(colorMode);
	
	LayerRecord lrRecord = LayerRecord(
		lrName,
		top,
		left,
		bottom,
		right,
		channelCount,
		channelInfoVec,
		Layer<T>::m_BlendMode,
		Layer<T>::m_Opacity,
		clipping,
		bitFlags,
		lrMaskData,
		blendingRanges,
		std::nullopt	// We dont really need to pass any additional layer info in here
	);

	return std::make_tuple(std::move(lrRecord), std::move(channelImgData));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
ImageLayer<T>::ImageLayer(const LayerRecord& layerRecord, ChannelImageData& channelImageData, const FileHeader& header) :
	Layer<T>(layerRecord, channelImageData, header)
{
	// Move the layers into our own layer representation
	for (int i = 0; i < layerRecord.m_ChannelCount; ++i)
	{
		auto& channelInfo = layerRecord.m_ChannelInformation[i];

		// We already extract masks ahead of time and skip them here to avoid raising warnings
		if (channelInfo.m_ChannelID.id == Enum::ChannelID::UserSuppliedLayerMask) continue;

		auto channelPtr = channelImageData.extractImagePtr(channelInfo.m_ChannelID);
		// Pointers might have already been
		if (!channelPtr) continue;

		// Insert any valid pointers to channels we have. We move to avoid having 
		// to uncompress / recompress
		if (auto imageChannelPtr = dynamic_cast<ImageChannel<T>*>(channelPtr.get()))
		{
			m_ImageData[channelInfo.m_ChannelID] = std::move(*imageChannelPtr);
		}
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::tuple<std::vector<LayerRecords::ChannelInformation>, ChannelImageData> ImageLayer<T>::extractImageData(const bool doCopy)
{
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

	// Extract all the channels next and push them into our data representation
	for (const auto& it : m_ImageData)
	{
		channelInfoVec.push_back(LayerRecords::ChannelInformation{ it.first, it.second.m_OrigByteSize });
		if (doCopy)
			channelDataVec.push_back(std::make_unique<ImageChannel<T>>(it.second));
		else
			channelDataVec.push_back(std::make_unique<ImageChannel<T>>(std::move(it.second)));
	}

	// Construct the channel image data from our vector of ptrs, moving gets handled by the constructor
	ChannelImageData channelData(std::move(channelDataVec));

	return std::make_tuple(channelInfoVec, std::move(channelData));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
ImageLayer<T>::ImageLayer(std::unordered_map<Enum::ChannelID, std::vector<T>>&& imageData, const Layer<T>::Params& layerParameters)
{
	PROFILE_FUNCTION();
	Layer<T>::m_LayerName = layerParameters.layerName;
	if (layerParameters.blendMode == Enum::BlendMode::Passthrough)
	{
		PSAPI_LOG_WARNING("ImageLayer", "The Passthrough blend mode is reserved for groups, defaulting to 'Normal'");
		Layer<T>::m_BlendMode = Enum::BlendMode::Normal;
	}
	else
	{
		Layer<T>::m_BlendMode = layerParameters.blendMode;
	}
	Layer<T>::m_Opacity = layerParameters.opacity;
	Layer<T>::m_IsVisible = true;
	Layer<T>::m_CenterX = layerParameters.posX;
	Layer<T>::m_CenterY = layerParameters.posY;
	Layer<T>::m_Width = layerParameters.width;
	Layer<T>::m_Height = layerParameters.height;

	// Construct a ChannelIDInfo for each of the channels and then an ImageLayer instance to hold the data
	for (auto& pair : imageData)
	{
		Enum::ChannelIDInfo info = {};
		if (layerParameters.colorMode == Enum::ColorMode::RGB)
			info = Enum::rgbChannelIDToChannelIDInfo(pair.first);
		else if (layerParameters.colorMode == Enum::ColorMode::CMYK)
			info = Enum::cmykChannelIDToChannelIDInfo(pair.first);
		else if (layerParameters.colorMode == Enum::ColorMode::Grayscale)
			info = Enum::grayscaleChannelIDToChannelIDInfo(pair.first);
		else
			PSAPI_LOG_ERROR("ImageLayer", "Currently PhotoshopAPI only supports RGB, CMYK and Grayscale ColorMode");

		// Channel sizes must match the size of the layer
		if (static_cast<uint64_t>(layerParameters.width) * layerParameters.height > pair.second.size()) [[unlikely]]
		{
			PSAPI_LOG_ERROR("ImageLayer", "Size of ImageChannel does not match the size of width * height, got %" PRIu64 " but expected %" PRIu64 ".",
				pair.second.size(),
				static_cast<uint64_t>(layerParameters.width) * layerParameters.height);
		}

			ImageChannel<T> channel = ImageChannel<T>(layerParameters.compression, std::move(pair.second), info, layerParameters.width, layerParameters.height, layerParameters.posX, layerParameters.posY);
			m_ImageData[info] = std::move(channel);
	}

	// Check that the required keys are actually present. e.g. for an RGB colorMode the channels R, G and B must be present
	if (layerParameters.colorMode == Enum::ColorMode::RGB)
	{
		Enum::ChannelIDInfo channelR = { .id = Enum::ChannelID::Red, .index = 0 };
		Enum::ChannelIDInfo channelG = { .id = Enum::ChannelID::Green, .index = 1 };
		Enum::ChannelIDInfo channelB = { .id = Enum::ChannelID::Blue, .index = 2 };
		std::vector<Enum::ChannelIDInfo> channelVec = { channelR, channelG, channelB };
		bool hasRequiredChannels = checkChannelKeys(m_ImageData, channelVec);
		if (!hasRequiredChannels)
		{
			PSAPI_LOG_ERROR("ImageLayer", "For RGB ColorMode R, G and B channels need to be specified");
		}
	}
	else if (layerParameters.colorMode == Enum::ColorMode::CMYK)
	{
		Enum::ChannelIDInfo channelC = { .id = Enum::ChannelID::Cyan, .index = 0 };
		Enum::ChannelIDInfo channelM = { .id = Enum::ChannelID::Magenta, .index = 1 };
		Enum::ChannelIDInfo channelY = { .id = Enum::ChannelID::Yellow, .index = 2 };
		Enum::ChannelIDInfo channelK = { .id = Enum::ChannelID::Black, .index = 3 };
		std::vector<Enum::ChannelIDInfo> channelVec = { channelC, channelM, channelY, channelK };
		bool hasRequiredChannels = checkChannelKeys(m_ImageData, channelVec);
		if (!hasRequiredChannels)
		{
			PSAPI_LOG_ERROR("ImageLayer", "For CMYK ColorMode C, M, Y and K channels need to be specified");
		}
	}
	else if (layerParameters.colorMode == Enum::ColorMode::Grayscale)
	{
		Enum::ChannelIDInfo channelG = { .id = Enum::ChannelID::Gray, .index = 0 };
		std::vector<Enum::ChannelIDInfo> channelVec = { channelG };
		bool hasRequiredChannels = checkChannelKeys(m_ImageData, { channelG });
		if (!hasRequiredChannels)
		{
			PSAPI_LOG_ERROR("ImageLayer", "For Grayscale ColorMode Gray channel needs to be specified");
		}
	}
	

	// Set the layer mask
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
ImageLayer<T>::ImageLayer(std::unordered_map<uint16_t, std::vector<T>>&& imageData, const Layer<T>::Params& layerParameters)
{
	PROFILE_FUNCTION();
	Layer<T>::m_LayerName = layerParameters.layerName;
	if (layerParameters.blendMode == Enum::BlendMode::Passthrough)
	{
		PSAPI_LOG_WARNING("ImageLayer", "The Passthrough blend mode is reserved for groups, defaulting to 'Normal'");
		Layer<T>::m_BlendMode = Enum::BlendMode::Normal;
	}
	else
	{
		Layer<T>::m_BlendMode = layerParameters.blendMode;
	}
	Layer<T>::m_Opacity = layerParameters.opacity;
	Layer<T>::m_IsVisible = true;
	Layer<T>::m_CenterX = layerParameters.posX;
	Layer<T>::m_CenterY = layerParameters.posY;
	Layer<T>::m_Width = layerParameters.width;
	Layer<T>::m_Height = layerParameters.height;

	// Construct a ChannelIDInfo for each of the channels and then an ImageLayer instance to hold the data
	for (auto& pair : imageData)
	{
		Enum::ChannelIDInfo info = {};
		if (layerParameters.colorMode == Enum::ColorMode::RGB)
			info = Enum::rgbIntToChannelID(pair.first);
		else if (layerParameters.colorMode == Enum::ColorMode::CMYK)
			info = Enum::cmykIntToChannelID(pair.first);
		else if (layerParameters.colorMode == Enum::ColorMode::Grayscale)
			info = Enum::grayscaleIntToChannelID(pair.first);
		else
			PSAPI_LOG_ERROR("ImageLayer", "Currently PhotoshopAPI only supports RGB, CMYK and Grayscale ColorMode");

		// Channel sizes must match the size of the layer
		if (static_cast<uint64_t>(layerParameters.width) * layerParameters.height > pair.second.size()) [[unlikely]]
		{
			PSAPI_LOG_ERROR("ImageLayer", "Size of ImageChannel does not match the size of width * height, got %" PRIu64 " but expected %" PRIu64 ".",
				pair.second.size(),
				static_cast<uint64_t>(layerParameters.width) * layerParameters.height);
		}

		ImageChannel<T> channel = ImageChannel<T>(layerParameters.compression, std::move(pair.second), info, layerParameters.width, layerParameters.height, layerParameters.posX, layerParameters.posY);
		m_ImageData[info] = std::move(channel);
	}

	// Check that the required keys are actually present. e.g. for an RGB colorMode the channels R, G and B must be present
	if (layerParameters.colorMode == Enum::ColorMode::RGB)
	{
		Enum::ChannelIDInfo channelR = { .id = Enum::ChannelID::Red, .index = 0 };
		Enum::ChannelIDInfo channelG = { .id = Enum::ChannelID::Green, .index = 1 };
		Enum::ChannelIDInfo channelB = { .id = Enum::ChannelID::Blue, .index = 2 };
		std::vector<Enum::ChannelIDInfo> channelVec = { channelR, channelG, channelB};
		bool hasRequiredChannels = checkChannelKeys(m_ImageData, channelVec);
		if (!hasRequiredChannels)
		{
			PSAPI_LOG_ERROR("ImageLayer", "For RGB ColorMode R, G and B channels need to be specified");
		}
	}
	else if (layerParameters.colorMode == Enum::ColorMode::CMYK)
	{
		Enum::ChannelIDInfo channelC = { .id = Enum::ChannelID::Cyan, .index = 0 };
		Enum::ChannelIDInfo channelM = { .id = Enum::ChannelID::Magenta, .index = 1 };
		Enum::ChannelIDInfo channelY = { .id = Enum::ChannelID::Yellow, .index = 2 };
		Enum::ChannelIDInfo channelK = { .id = Enum::ChannelID::Black, .index = 3 };
		std::vector<Enum::ChannelIDInfo> channelVec = { channelC, channelM, channelY, channelK };
		bool hasRequiredChannels = checkChannelKeys(m_ImageData, channelVec);
		if (!hasRequiredChannels)
		{
			PSAPI_LOG_ERROR("ImageLayer", "For CMYK ColorMode C, M, Y and K channels need to be specified");
		}
	}
	else if (layerParameters.colorMode == Enum::ColorMode::Grayscale)
	{
		Enum::ChannelIDInfo channelG = { .id = Enum::ChannelID::Gray, .index = 0 };
		std::vector<Enum::ChannelIDInfo> channelVec = { channelG };
		bool hasRequiredChannels = checkChannelKeys(m_ImageData, { channelG });
		if (!hasRequiredChannels)
		{
			PSAPI_LOG_ERROR("ImageLayer", "For Grayscale ColorMode Gray channel needs to be specified");
		}
	}


	// Set the layer mask
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
void ImageLayer<T>::setCompression(const Enum::Compression compCode)
{
	// Change the mask channels' compression codec
	Layer<T>::setCompression(compCode);
	// Change the image channel compression codecs
	for (const auto& [key, val] : m_ImageData)
	{
		m_ImageData[key].m_Compression = compCode;
	}
}


PSAPI_NAMESPACE_END