/*
Example of creating a layered file with groups through the PhotoshopAPI. The preferred method is always to interact through the LayeredFile
struct rather than directly with the low level PhotoshopAPI::PhotoshopFile although that is also possible but requires a deep understanding of the
underlying structure whereas LayeredFile abstracts the implementation details.
*/

#include "PhotoshopAPI.h"

#include <unordered_map>
#include <vector>


int main()
{
	// Initialize some constants that we will need throughout the program
	const static uint32_t width = 64u;
	const static uint32_t height = 64u;

	// Create an 8-bit LayeredFile as our starting point, 8- 16- and 32-bit are fully supported
	PhotoshopAPI::LayeredFile<PhotoshopAPI::bpp8_t> document = { PhotoshopAPI::Enum::ColorMode::RGB, width, height };
	
	PhotoshopAPI::GroupLayer<PhotoshopAPI::bpp8_t>::Params groupParams = {};
	groupParams.layerName = "Group";
	// We dont need to specify a width or height if we do not have a mask channel

	// As with image layers we can first add the group to the document root and modify the group after 
	auto groupLayer = std::make_shared<PhotoshopAPI::GroupLayer<PhotoshopAPI::bpp8_t>>(groupParams);
	document.addLayer(groupLayer);

	// Create an image layer and insert it under the group
	{
		std::unordered_map <PhotoshopAPI::Enum::ChannelID, std::vector<PhotoshopAPI::bpp8_t>> channelMap;
		channelMap[PhotoshopAPI::Enum::ChannelID::Red] = std::vector<PhotoshopAPI::bpp8_t>(width * height, 255u);
		channelMap[PhotoshopAPI::Enum::ChannelID::Green] = std::vector<PhotoshopAPI::bpp8_t>(width * height, 0u);
		channelMap[PhotoshopAPI::Enum::ChannelID::Blue] = std::vector<PhotoshopAPI::bpp8_t>(width * height, 0u);

		PhotoshopAPI::ImageLayer<PhotoshopAPI::bpp8_t>::Params layerParams = {};
		layerParams.layerName = "Layer Red";
		layerParams.width = width;
		layerParams.height = height;

		auto layer = std::make_shared<PhotoshopAPI::ImageLayer<PhotoshopAPI::bpp8_t>>(
			std::move(channelMap),
			layerParams
		);
		// Adding the layer twice would be invalid and would raise a warning as each layer needs to be created uniquely
		// document.addLayer(layer);
		groupLayer->addLayer(document, layer);
	}

	// Convert to PhotoshopDocument and write to disk. Note that from this point onwards 
	// our LayeredFile instance is no longer usable
	PhotoshopAPI::File::FileParams params = { .doRead = false, .forceOverwrite = true };
	auto outputFile = PhotoshopAPI::File("./WriteGroupedFile.psd", params);
	auto psdDocumentPtr = PhotoshopAPI::LayeredToPhotoshopFile(std::move(document));
	psdDocumentPtr->write(outputFile);
}