#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Layer.h"
#include "Struct/ImageChannel.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"

#include <unordered_map>

PSAPI_NAMESPACE_BEGIN


// A pixel based image layer
template <typename T>
struct ImageLayer : public Layer<T>
{
	// Store the image data as a per-channel map to be used later using a custom hash function
	std::unordered_map<Enum::ChannelIDInfo, ImageChannel<T>, Enum::ChannelIDInfoHasher> m_ImageData;

	// Generate a photoshop layerRecord and imageData based on the current layer. if doCopy is set to false this will likely
	// invalidate both our m_ImageData as well as our m_LayerMask since we perform move operations on them. If doCopy is 
	// set to true we can safely keep using the ImageLayer instance. it is advised to only set doCopy to false on parsing of
	// the whole layeredFile -> PhotoshopFile.
	std::tuple<LayerRecord, ChannelImageData> toPhotoshop(const Enum::ColorMode colorMode, const bool doCopy, const FileHeader& header) override;

	// Initialize our imageLayer by first parsing the base Layer instance and then moving
	// the additional channels into our representation
	ImageLayer(const LayerRecord& layerRecord, ChannelImageData& channelImageData);


	/// <summary>
	/// Generate an ImageLayer instance ready to be used in a LayeredFile document.
	/// </summary>
	/// <param name="imageData">The actual image data for the layer. If custom channels are to be used please use the Constructor with uint16_t as key instead</param>
	/// <param name="maskData">An optional layer mask. currently only Pixel masks are supported</param>
	/// <param name="layerName">The name of the layer. This parameter will not automatically insert groups if separated by </param>
	/// <param name="blendMode"></param>
	/// <param name="posX">The position of the center of the Image. 0 equals to the image center</param>
	/// <param name="posY">The position of the center of the Image. 0 equals to the image center</param>
	/// <param name="width">The absolute width of the image.</param>
	/// <param name="height">The absolute height of the image.</param>
	/// <param name="compression">The compression mode with which the file gets saved to disk, keep at default for best compression</param>
	/// <param name="colorMode">The colorMode the layeredFile is in</param>
	ImageLayer(
		std::unordered_map<Enum::ChannelID, std::vector<T>>&& imageData,
		std::optional<std::vector<T>>&& maskData,
		const std::string layerName,
		const Enum::BlendMode blendMode,
		const int32_t posX,
		const int32_t posY,
		const uint32_t width,
		const uint32_t height,
		const Enum::Compression compression = Enum::Compression::ZipPrediction,
		const Enum::ColorMode colorMode = Enum::ColorMode::RGB
	);

	/// <summary>
	/// Generate an ImageLayer instance ready to be used in a LayeredFile document.
	/// </summary>
	/// <param name="imageData">The actual image data for the layer</param>
	/// <param name="maskData">An optional layer mask. currently only Pixel masks are supported</param>
	/// <param name="layerName">The name of the layer. This parameter will not automatically insert groups if separated by </param>
	/// <param name="blendMode"></param>
	/// <param name="posX">The position of the center of the Image. 0 equals to the image center</param>
	/// <param name="posY">The position of the center of the Image. 0 equals to the image center</param>
	/// <param name="width">The absolute width of the image.</param>
	/// <param name="height">The absolute height of the image.</param>
	/// <param name="compression">The compression mode with which the file gets saved to disk, keep at default for best compression</param>
	/// <param name="colorMode">The colorMode the layeredFile is in</param>
	ImageLayer(
		std::unordered_map<uint16_t, std::vector<T>>&& imageData,
		std::optional<std::vector<T>>&& maskData,
		const std::string layerName,
		const Enum::BlendMode blendMode,
		const int32_t posX,
		const int32_t posY,
		const uint32_t width,
		const uint32_t height,
		const Enum::Compression compression = Enum::Compression::ZipPrediction,
		const Enum::ColorMode colorMode = Enum::ColorMode::RGB
	);

private:
	// Extracts the m_ImageData as well as the layer mask into two vectors holding channel information as well as the image data 
	// itself. This also takes care of generating our layer mask channel if it is present
	std::tuple<std::vector<LayerRecords::ChannelInformation>, ChannelImageData> extractImageData(const bool doCopy);
};

PSAPI_NAMESPACE_END