#include "PhotoshopAPI.h"


int main()
{
	// Initialize some constants that we will need throughout the program
	const static PhotoshopAPI::Enum::ColorMode colorMode = PhotoshopAPI::Enum::ColorMode::RGB;
	const static uint32_t width = 64u;
	const static uint32_t height = 64u;

	PhotoshopAPI::LayeredFile<uint8_t> document = { colorMode, width, height };

	// Create our individual channels to add to our image layer. Keep in mind that all these 3 channels need to 
	// be specified for RGB mode
	std::unordered_map <PhotoshopAPI::Enum::ChannelID, std::vector<uint8_t>> channelMap;
	channelMap[PhotoshopAPI::Enum::ChannelID::Red] = std::vector<uint8_t>(width * height, 255u);
	channelMap[PhotoshopAPI::Enum::ChannelID::Green] = std::vector<uint8_t>(width * height, 0u);
	channelMap[PhotoshopAPI::Enum::ChannelID::Blue] = std::vector<uint8_t>(width * height, 0u);

	auto layer = std::make_shared<PhotoshopAPI::ImageLayer<uint8_t>>(
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

	// It is perfectly legal to modify a layers properties even after it was added to the document
	layer->m_Opacity = 128u;

	// Convert to PhotoshopDocument and write to disk. Note that from this point onwards 
	// our LayeredFile instance is no longer usable
	auto outputFile = PhotoshopAPI::File("./WriteSimpleFile.psd", false, true);
	auto psdDocumentPtr = document.toPhotoshopFile();
	psdDocumentPtr->write(outputFile);
}