/*
Example of creating a layered file with groups through the PhotoshopAPI.
*/

#include "PhotoshopAPI.h"

#include <unordered_map>
#include <vector>


int main()
{
	using namespace NAMESPACE_PSAPI;

	// Initialize some constants that we will need throughout the program
	static constexpr uint32_t width = 64u;
	static constexpr uint32_t height = 64u;

	// Create an 8-bit LayeredFile as our starting point, 8- 16- and 32-bit are fully supported
	LayeredFile<bpp8_t> document = { Enum::ColorMode::RGB, width, height };
	
	GroupLayer<bpp8_t>::Params group_params = {};
	group_params.name = "Group";
	// We don't need to specify a width or height if we do not have a mask channel

	// As with image layers we can first add the group to the document root and modify the group after 
	auto groupLayer = std::make_shared<GroupLayer<bpp8_t>>(group_params);
	document.add_layer(groupLayer);

	// Create an image layer and insert it under the group
	{
		std::unordered_map <Enum::ChannelID, std::vector<bpp8_t>> channel_map;
		channel_map[Enum::ChannelID::Red] = std::vector<bpp8_t>(width * height, 255u);
		channel_map[Enum::ChannelID::Green] = std::vector<bpp8_t>(width * height, 0u);
		channel_map[Enum::ChannelID::Blue] = std::vector<bpp8_t>(width * height, 0u);

		ImageLayer<bpp8_t>::Params layer_params = {};
		layer_params.name = "Layer Red";
		layer_params.width = width;
		layer_params.height = height;

		auto layer = std::make_shared<ImageLayer<bpp8_t>>(
			std::move(channel_map),
			layer_params
		);
		// Adding the layer twice would be invalid and would raise a warning as each layer needs to be created uniquely
		// document.addLayer(layer);
		groupLayer->add_layer(document, layer);
	}

	// Convert to PhotoshopDocument and write to disk. Note that from this point onwards 
	// our LayeredFile instance is no longer usable
	LayeredFile<bpp8_t>::write(std::move(document), "WriteGroupedFile.psd");
}