#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Layer.h"
#include "Core/Struct/ImageChannel.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"

#include <unordered_map>

PSAPI_NAMESPACE_BEGIN


// A pixel based image layer
template <typename T>
struct ImageLayer : public Layer<T>
{
	/// Store the image data as a per-channel map to be used later using a custom hash function
	std::unordered_map<Enum::ChannelIDInfo, std::unique_ptr<ImageChannel>, Enum::ChannelIDInfoHasher> m_ImageData;

	/// Generate a photoshop layerRecord and imageData based on the current layer
	std::tuple<LayerRecord, ChannelImageData> toPhotoshop(const Enum::ColorMode colorMode, const FileHeader& header) override;

	/// Initialize our imageLayer by first parsing the base Layer instance and then moving
	/// the additional channels into our representation
	ImageLayer(const LayerRecord& layerRecord, ChannelImageData& channelImageData, const FileHeader& header);

	/// Generate an ImageLayer instance ready to be used in a LayeredFile document.
	ImageLayer(std::unordered_map<Enum::ChannelID, std::vector<T>>&& imageData, Layer<T>::Params& layerParameters);

	/// Generate an ImageLayer instance ready to be used in a LayeredFile document.
	ImageLayer(std::unordered_map<uint16_t, std::vector<T>>&& imageData, Layer<T>::Params& layerParameters);

	/// Extract a specified channel from the layer given its channel ID. This also works for masks
	///
	/// \param channelID the channel ID to extract
	/// \param doCopy whether to extract the channel by copying the data. If this is false the channel will no longer hold any image data!
	std::vector<T> getChannel(const Enum::ChannelID channelID, bool doCopy = true);

	/// Extract a specified channel from the layer given its channel ID. This also works for masks
	///
	/// \param channelIndex the channel index to extract
	/// \param doCopy whether to extract the channel by copying the data. If this is false the channel will no longer hold any image data!
	std::vector<T> getChannel(const int16_t channelIndex, bool doCopy = true);

	/// Extract all the channels of the ImageLayer into an unordered_map. Includes the mask channel
	/// 
	/// \param doCopy whether to extract the image data by copying the data. If this is false the channel will no longer hold any image data!
	std::unordered_map<Enum::ChannelIDInfo, std::vector<T>, Enum::ChannelIDInfoHasher> getImageData(bool doCopy = true);

	/// Change the compression codec of all the image channels
	void setCompression(const Enum::Compression compCode);

private:
	// Extracts the m_ImageData as well as the layer mask into two vectors holding channel information as well as the image data 
	// itself. This also takes care of generating our layer mask channel if it is present. Invalidates any data held by the ImageLayer
	std::tuple<std::vector<LayerRecords::ChannelInformation>, ChannelImageData> generateChannelImageData();
};

PSAPI_NAMESPACE_END