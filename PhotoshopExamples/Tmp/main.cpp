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

	std::vector<uint8_t> interleaved = NAMESPACE_PSAPI::Render::interleave_alloc(r_span, g_span, b_span);

	// Set up the image specification
	ImageSpec spec(width, height, 3, TypeDesc::UINT8);
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
	
	// Render surface
	{
		auto mesh = layer_ptr->warp().mesh();
		mesh.move({ -mesh.bbox().minimum.x, -mesh.bbox().minimum.y });

		std::vector<uint8_t> data(static_cast<size_t>(mesh.bbox().width()) * static_cast<size_t>(mesh.bbox().height()));
		Render::ImageBuffer<uint8_t> buffer(data, static_cast<size_t>(mesh.bbox().width()), static_cast<size_t>(mesh.bbox().height()));
		Render::render_mesh<uint8_t, double>(buffer, mesh, 255);

		write_to_disk(data, "C:/Users/emild/Desktop/linkedlayers/warp/warpsurface.png", buffer.width, buffer.height);
	}


	// Render mesh
	{
		auto mesh = layer_ptr->warp().surface().mesh(9, 9, true);

		std::vector<uint8_t> data(static_cast<size_t>(mesh.bbox().width()) * static_cast<size_t>(mesh.bbox().height()));
		Render::ImageBuffer<uint8_t> buffer(data, static_cast<size_t>(mesh.bbox().width()), static_cast<size_t>(mesh.bbox().height()));
		Render::render_mesh<uint8_t, double>(buffer, mesh, 255);

		write_to_disk(data, "C:/Users/emild/Desktop/linkedlayers/warp/warpmesh.png", buffer.width, buffer.height);
	}

	// Render image data
	{
		auto orig_image_data = layer_ptr->original_image_data();
		auto image_data = layer_ptr->get_image_data();
		write_to_disk(orig_image_data, "C:/Users/emild/Desktop/linkedlayers/warp/original.png", layer_ptr->original_width(), layer_ptr->original_height());
		write_to_disk(image_data, "C:/Users/emild/Desktop/linkedlayers/warp/warped.png", layer_ptr->width(), layer_ptr->height());
	}

	layer_ptr->replace("C:/Users/emild/Desktop/linkedlayers/warp/uv_grid.jpg");

	// Render image data replaced
	{
		auto orig_image_data = layer_ptr->original_image_data();
		auto image_data = layer_ptr->get_image_data();
		write_to_disk(orig_image_data, "C:/Users/emild/Desktop/linkedlayers/warp/original_replaced.png", layer_ptr->original_width(), layer_ptr->original_height());
		write_to_disk(image_data, "C:/Users/emild/Desktop/linkedlayers/warp/warped_replaced.png", layer_ptr->width(), layer_ptr->height());
	}

	Instrumentor::Get().EndSession(); 
}