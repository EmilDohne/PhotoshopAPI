/*
Example of loading a PhotoshopFile and extracting the image data, this can be freely used with any other operations present on the LayeredFile
*/
#include <OpenImageIO/imageio.h>
#include "PhotoshopAPI.h"
#include "Core/Render/Render.h"
#include "Core/Render/Interleave.h"

#include <unordered_map>
#include <vector>
#include <string>
#include <span>
#include <format>

using namespace NAMESPACE_PSAPI;

// Helper function to convert Enum::ChannelIDInfo to a string
std::string channel_id_to_string(Enum::ChannelIDInfo channel_id) 
{
	switch (channel_id.id) {
	case Enum::ChannelID::Red: return "R";
	case Enum::ChannelID::Green: return "G";
	case Enum::ChannelID::Blue: return "B";
	case Enum::ChannelID::Alpha: return "A";
	default: return "Unknown";
	}
}


void write_to_disk(const std::vector<uint8_t> channel,
	const std::string& output_filename,
	int width, int height)
{
	using namespace OIIO;

	// Open the file for writing
	std::unique_ptr<ImageOutput> out = ImageOutput::create(output_filename);
	if (!out) {
		throw std::runtime_error("Failed to create output file: " + output_filename);
	}

	// Set up the image specification
	ImageSpec spec(width, height, 1, TypeDesc::UINT8);

	// Open the file with the specification
	if (!out->open(output_filename, spec)) {
		throw std::runtime_error("Failed to open file: " + out->geterror());
	}
	// Write the image
	if (!out->write_image(TypeDesc::UINT8, channel.data())) {
		throw std::runtime_error("Failed to write image: " + out->geterror());
	}

	// Close the file
	out->close();
}


void write_to_disk(const std::unordered_map<Enum::ChannelIDInfo, std::vector<uint8_t>, Enum::ChannelIDInfoHasher>& channel_map,
	const std::string& output_filename,
	int width, int height)
{
	using namespace OIIO;

	// Open the file for writing
	std::unique_ptr<ImageOutput> out = ImageOutput::create(output_filename);
	if (!out) 
	{
		throw std::runtime_error("Failed to create output file: " + output_filename);
	}

	// Determine the number of channels and validate dimensions
	int num_channels = static_cast<int>(channel_map.size());
	if (num_channels == 0) 
	{
		throw std::runtime_error("Channel map is empty, nothing to write!");
	}

	// Prepare the channel data
	std::vector<std::string> channel_names;
	if (channel_map.size() == 3)
	{
		channel_names = { "R", "G", "B" };
	}
	else if (channel_map.size() == 4)
	{
		channel_names = { "R", "G", "B", "A"};
	}

	Enum::ChannelIDInfo red = { Enum::ChannelID::Red, 0 };
	std::span<const uint8_t> r_span = std::span<const uint8_t>(channel_map.at(red).begin(), channel_map.at(red).end());
	Enum::ChannelIDInfo green = { Enum::ChannelID::Green, 1 };
	std::span<const uint8_t> g_span = std::span<const uint8_t>(channel_map.at(green).begin(), channel_map.at(green).end());
	Enum::ChannelIDInfo blue = { Enum::ChannelID::Blue, 2 };
	std::span<const uint8_t> b_span = std::span<const uint8_t>(channel_map.at(blue).begin(), channel_map.at(blue).end());
	Enum::ChannelIDInfo alpha = { Enum::ChannelID::Alpha, -1 };
	std::span<const uint8_t> a_span = std::span<const uint8_t>(channel_map.at(alpha).begin(), channel_map.at(alpha).end());

	std::vector<uint8_t> interleaved = NAMESPACE_PSAPI::Render::interleave_alloc(r_span, g_span, b_span, a_span);

	// Set up the image specification
	ImageSpec spec(width, height, 4, TypeDesc::UINT8);
	spec.channelnames = channel_names;

	// Open the file with the specification
	if (!out->open(output_filename, spec)) {
		throw std::runtime_error("Failed to open file: " + out->geterror());
	}
	// Write the image
	if (!out->write_image(TypeDesc::UINT8, interleaved.data())) {
		throw std::runtime_error("Failed to write image: " + out->geterror());
	}

	// Close the file
	out->close();
}


int main()
{
	using namespace NAMESPACE_PSAPI;

	Instrumentor::Get().BeginSession("Tmp", "Tmp.json");

	LayeredFile<bpp8_t> file = LayeredFile<bpp8_t>::read("C:/Users/emild/Desktop/linkedlayers/warp/warp_tmp.psd");
	auto layer_ptr = find_layer_as<bpp8_t, SmartObjectLayer>("WarpQuilt", file);
	layer_ptr->replace("C:/Users/emild/Desktop/uv_grid.jpg");
	layer_ptr->set_linkage(LinkedLayerType::external);

	auto params = Layer<bpp8_t>::Params{};
	params.name = "UVGrid";
	auto layer_new = std::make_shared<SmartObjectLayer<bpp8_t>>(file, params, "C:/Users/emild/Desktop/linkedlayers/warp/uv_grid.jpg");
	layer_new->warp(layer_ptr->warp());

	file.add_layer(layer_new);

	LayeredFile<bpp8_t>::write(std::move(file), "C:/Users/emild/Desktop/linkedlayers/warp/warp_tmp_out1.psd");


	Instrumentor::Get().EndSession();
}