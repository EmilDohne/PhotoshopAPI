/*
Example of loading a PhotoshopFile and extracting the image data, modifying this and then replacing it again
*/

#include "PhotoshopAPI.h"

#include <algorithm>
#include <execution>
#include <cmath>
#include <cstdint>
#include <vector>
#include <unordered_map>


// sRGB to linear conversion
float srgbToLinear(uint8_t srgb)
{
	float normalized = srgb / 255.0f;
	if (normalized <= 0.04045f)
		return normalized / 12.92f;
	else
		return std::pow((normalized + 0.055f) / 1.055f, 2.4f);
}

int main()
{
	using namespace NAMESPACE_PSAPI;

	// In this case we already know the bit depth but otherwise one could use the PhotoshopFile.m_Header.m_Depth
	// variable on the PhotoshopFile to figure it out programmatically. This would need to be done using the 
	// "extended" read signature shown in the ExtendedSignature example.
	LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read("ImageData.psb");

	// We could also use find_layer() on the LayeredFile but this way we directly get the appropriate type.
	// Keep in mind this can return nullptr!
	auto imageLayerPtr = find_layer_as<bpp8_t, ImageLayer>("Blue_Lagoon/Blue_Lagoon.exr", layeredFile);

	// Now we can grab all channels (we could also use just grab a single channel)
	auto channels = imageLayerPtr->get_image_data();

	// Now we do our modifications. In this example we apply a sRGB -> linear operation
	for (auto& [_, value] : channels)
	{
		std::for_each(std::execution::par, value.begin(), value.end(), [&](uint8_t& pixelValue) 
			{
				pixelValue = static_cast<uint8_t>(std::round(srgbToLinear(pixelValue) * 255.0f));
			});
	}

	// Finally we can set the image data to the channel again and save out our file
	imageLayerPtr->set_image_data(std::move(channels));
	LayeredFile<bpp8_t>::write(std::move(layeredFile), "ModifiedImageData.psb");
}