/*
Example of creating a simple document with a single layer using the PhotoshopAPI. 
*/

#include "PhotoshopAPI.h"

#include <unordered_map>
#include <vector>


int main()
{
	using namespace NAMESPACE_PSAPI;

	// Initialize some constants that we will need throughout the program
	const static uint32_t width = 64u;
	const static uint32_t height = 64u;

	// Create an 8-bit LayeredFile as our starting point, 8- 16- and 32-bit are fully supported
	LayeredFile<bpp8_t> document = { Enum::ColorMode::RGB, width, height };
	// Create our individual channels to add to our image layer. Keep in mind that all these 3 channels need to 
	// be specified for RGB mode
	std::unordered_map <Enum::ChannelID, std::vector<bpp8_t>> channelMap;
	channelMap[Enum::ChannelID::Red] = std::vector<bpp8_t>(width * height, 255u);
	channelMap[Enum::ChannelID::Green] = std::vector<bpp8_t>(width * height, 0u);
	channelMap[Enum::ChannelID::Blue] = std::vector<bpp8_t>(width * height, 0u);

	ImageLayer<bpp8_t>::Params layerParams = {};
	layerParams.name = "Layer Red";
	layerParams.width = width;
	layerParams.height = height;

	auto layer = std::make_shared<ImageLayer<bpp8_t>>(std::move(channelMap), layerParams);
	document.add_layer(layer);

	// It is perfectly legal to modify a layers properties even after it was added to the document as attributes
	// are only finalized on export
	layer->opacity(.5);

	// Convert to PhotoshopFile and write to disk. Note that from this point onwards 
	// our LayeredFile instance is no longer usable
	LayeredFile<bpp8_t>::write(std::move(document), "WriteSimpleFile.psd");
}