/*
Example of loading a PhotoshopFile and extracting the image data, this can be freely used with any other operations present on the LayeredFile
*/

#include "PhotoshopAPI.h"

#include <unordered_map>
#include <vector>


int main()
{
	using namespace NAMESPACE_PSAPI;

	// In this case we already know the bit depth but otherwise one could use the PhotoshopFile.m_Header.m_Depth
	// variable on the PhotoshopFile to figure it out programmatically. This would need to be done using the 
	// "extended" read signature shown in the ExtendedSignature example.
	LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read("ImageData.psb");

	// We could also use findLayer() on the LayeredFile but this way we directly get the appropriate type
	auto imageLayerPtr = findLayerAs<bpp8_t, ImageLayer>("RedLayer", layeredFile);

	// We can now either extract just the channels we want:
	std::vector<bpp8_t> channel_r = imageLayerPtr->getChannel(Enum::ChannelID::Red);
	std::vector<bpp8_t> channel_g = imageLayerPtr->getChannel(Enum::ChannelID::Green);
	std::vector<bpp8_t> channel_b = imageLayerPtr->getChannel(Enum::ChannelID::Blue);

	// or extract all the channels as one unordered_map:
	auto channels = imageLayerPtr->getImageData();

	// over which we could loop like this:
	for (auto& [key, value] : channels)
	{
		// key is a Enum::ChannelIDInfo object where the .id is a Enum::ChannelID 
		// and .index is an unsigned integer

		// value is simply a vector of type T, in our case a std::vector<bpp8_t>

		// This will also extract the channel mask if it exists
	}

	// If we want to extract e.g. the layer mask:
	auto maskImageLayerPtr = findLayerAs<bpp8_t, ImageLayer>("Group/EmptyLayerWithMask", layeredFile);

	// If this doesnt have a mask channel we will simply get an empty channel. In this case though, even though
	// we have a mask it will be empty as well as Photoshop fills in the gaps in the layer with the defaultColor
	// parameter.
	std::vector<bpp8_t> channel_mask = maskImageLayerPtr->getMaskData();

	// To extract this default color we can do this:
	if (maskImageLayerPtr->m_LayerMask.has_value())
	{
		// This value is always uint8_t even for 16- and 32- bit files!
		uint8_t defaultColor = maskImageLayerPtr->m_LayerMask.value().defaultColor;
	}
	// This would tell us that we have an empty white layer mask with no pixel values.
	// One can however write out explicit zeroes for mask channels or set a default color
}