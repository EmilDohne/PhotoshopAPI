#include "PhotoshopAPI.h"


int main()
{
	// Initialize some constants that we will need throughout the program
	const PhotoshopAPI::Enum::ColorMode colorMode = PhotoshopAPI::Enum::ColorMode::RGB;
	const uint32_t width = 64u;
	const uint32_t height = 64u;

	PhotoshopAPI::LayeredFile<uint8_t> document = PhotoshopAPI::LayeredFile<uint8_t>(colorMode, width, height);

	// Create our individual channels to add to our image layer. Keep in mind that all 3 channels need to be specified
	// for RGB mode
	std::unordered_map <PhotoshopAPI::Enum::ChannelID, std::vector<uint8_t>> channelMap;
	channelMap[PhotoshopAPI::Enum::ChannelID::Red] = std::vector<uint8_t>(static_cast<uint64_t>(width) * height, 255u);
	channelMap[PhotoshopAPI::Enum::ChannelID::Green] = std::vector<uint8_t>(static_cast<uint64_t>(width) * height, 0u);
	channelMap[PhotoshopAPI::Enum::ChannelID::Blue] = std::vector<uint8_t>(static_cast<uint64_t>(width) * height, 0u);

	// We use a shared_ptr as this allows us to keep modifying this layer if we wanted.
	std::shared_ptr<PhotoshopAPI::ImageLayer<uint8_t>> layer = std::make_shared<PhotoshopAPI::ImageLayer<uint8_t>>(
		std::move(channelMap),					// The image channels for our image layer
		std::nullopt,							// An optional mask channel
		"Red Layer",							// The name of the layer.
		PhotoshopAPI::Enum::BlendMode::Normal,	// The blend mode we wish to assign
		0u,										// X Center coordinate, 0 implies it is centered around the document
		0u,										// Y Center coordinate, 0 implies it is centered around the document
		width,						
		height,
		PhotoshopAPI::Enum::Compression::Zip
	);

	document.addLayer(layer);

	//Set some other properties on the layer. This will update the layer and we dont actually need to ask the LayeredFile instance for the layer again.
	layer->m_Opacity = 128u;

	// Convert to PhotoshopDocument and write to disk. Note that from this point onwards 
	// our LayeredFile instance is no longer usable
	auto outputFile = PhotoshopAPI::File("./WriteSimpleFile.psd", false, true);
	auto psdDocumentPtr = document.toPhotoshopFile();
	psdDocumentPtr->write(outputFile);
}