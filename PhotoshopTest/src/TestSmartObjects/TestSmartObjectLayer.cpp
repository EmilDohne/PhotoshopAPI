#include "doctest.h"

#include "PhotoshopAPI.h"
#include "Core/Struct/DescriptorStructure.h"
#include "LayeredFile/LayerTypes/SmartObjectLayer.h"
#include "Core/Warp/SmartObjectWarp.h"

#include "../DetectArmMac.h"

#include "Core/Render/ImageBuffer.h"
#include "Core/Render/Composite.h"
#include "Core/Render/Render.h"
#include "LayeredFile/LayeredFile.h"

#include <memory>
#include <iostream>
#include <filesystem>

#include <fmt/format.h>

#include <OpenImageIO/imagebufalgo.h>
#include <OpenImageIO/imagebuf.h>


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Create smartobject with path")
{
	using namespace NAMESPACE_PSAPI;
	using bpp_type = uint8_t;

	auto file = LayeredFile<bpp_type>(Enum::ColorMode::RGB, 64, 64);

	Layer<bpp_type>::Params lr_params{};
	lr_params.name = "SmartObject";
	lr_params.width = 64;
	lr_params.height = 32;

	auto layer = std::make_shared<SmartObjectLayer<bpp_type>>(file, lr_params, "documents/image_data/ImageStackerImage.jpg");
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("apply transformation to layer")
{
	using namespace NAMESPACE_PSAPI;
	using bpp_type = uint8_t;

	auto file = LayeredFile<bpp_type>(Enum::ColorMode::RGB, 64, 64);

	Layer<bpp_type>::Params lr_params{};
	lr_params.name = "SmartObject";
	lr_params.width = 64;
	lr_params.height = 32;

	auto layer = std::make_shared<SmartObjectLayer<bpp_type>>(file, lr_params, "documents/image_data/ImageStackerImage.jpg");
	layer->width(500);
	layer->height(250);


	CHECK(layer->width() == 500);
	CHECK(layer->height() == 250);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("modify warp and get dimensions")
{
	using namespace NAMESPACE_PSAPI;
	using bpp_type = uint8_t;

	auto file = LayeredFile<bpp_type>(Enum::ColorMode::RGB, 64, 64);

	Layer<bpp_type>::Params lr_params{};
	lr_params.name = "SmartObject";
	lr_params.width = 200;
	lr_params.height = 108;

	auto layer = std::make_shared<SmartObjectLayer<bpp_type>>(file, lr_params, "documents/image_data/ImageStackerImage.jpg");

	auto warp = layer->warp();
	auto warp_points = warp.points();
	warp_points[0] = warp_points[0] - 500;
	warp.points(warp_points);
	layer->warp(warp);

	// Since the warp is constrained by the transformation and not the mesh
	// we expect the same result.
	CHECK(layer->width() == doctest::Approx(200.0f));
	CHECK(layer->height() == doctest::Approx(108.0f));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Read all supported warps and write image files")
{
	using namespace NAMESPACE_PSAPI;
	using bpp_type = uint8_t;

	auto file = LayeredFile<bpp_type>::read(std::filesystem::current_path() / "documents/SmartObjects/smart_objects_transformed.psd");
	
	auto base_out_path = std::filesystem::current_path() / "documents/SmartObjects/out";
	auto base_ref_path = std::filesystem::current_path() / "documents/SmartObjects/reference";

	/// Compare two layers within the given tolerance
	auto compare_layer = [&base_ref_path](
		std::filesystem::path _reference_path,
		Render::ImageBuffer<bpp_type>& generated_warp
		)
		{
			auto inp = OIIO::ImageInput::open(_reference_path);
			CHECK(inp);
			const OIIO::ImageSpec& spec = inp->spec();
			int xres = spec.width;
			int yres = spec.height;
			int nchannels = spec.nchannels;
			auto pixels = std::vector<bpp_type>(xres * yres * nchannels);
			inp->read_image(0, 0, 0, nchannels, Render::get_type_desc<bpp_type>(), pixels.data());
			inp->close();

			CHECK(xres == generated_warp.width);
			CHECK(yres == generated_warp.height);
			CHECK(nchannels == generated_warp.num_channels());
			CHECK(nchannels == 4);

			auto deinterleaved = Render::deinterleave_alloc<bpp_type>(pixels, nchannels);
			std::unordered_map<int, Render::ChannelBuffer<bpp_type>> read_channels = {
				{ 0,  Render::ChannelBuffer<bpp_type>(deinterleaved[0], xres, yres) },
				{ 1,  Render::ChannelBuffer<bpp_type>(deinterleaved[1], xres, yres) },
				{ 2,  Render::ChannelBuffer<bpp_type>(deinterleaved[2], xres, yres) },
				{ -1, Render::ChannelBuffer<bpp_type>(deinterleaved[3], xres, yres) }
			};

			std::vector<int> indices = { 0, 1, 2, -1 };
			for (auto index : indices)
			{
				auto oiio_buffer_read = read_channels[index].to_oiio();
				auto oiio_buffer_created = generated_warp.channels[index].to_oiio();

				auto result = OIIO::ImageBufAlgo::compare(oiio_buffer_read, oiio_buffer_created, 255.0f, 255.0f);

				std::cout << _reference_path.string() << ": " << std::endl;
				std::cout << "\tImage differed: " << result.nfail << " failures, "
					<< result.nwarn << " warnings.\n";
				std::cout << "\tAverage error was " << result.meanerror << "\n";
				std::cout << "\tRMS error was " << result.rms_error << "\n";
				std::cout << "\tPSNR was " << result.PSNR << "\n";
				std::cout << "\tlargest error was " << result.maxerror
					<< " on pixel (" << result.maxx << "," << result.maxy
					<< "," << result.maxz << "), channel " << index << "\n";

				// We check for less than a .4% error, since our edges are fairly different
				// we cannot check for single pixels being below a certain tolerance.
				if (_reference_path == base_ref_path / "perspective_transform.png")
				{
					// This needs some extra clarification, photoshop when just using a perspective warp (no warp) changes the image
					// by warping the image similar to how an st map would do, not by actually warping the mesh defined by the 4 corners.
					// This seems to be an edge case and we don't intend to reproduce what photoshop does 1 to 1 because it is close enough
					// and would add a lot of overhead
					CHECK(result.meanerror < 0.01f);
				}
				else
				{
					CHECK(result.meanerror < 0.004f);
				}
			}
		};

	/// Check the warp and compare the layers
	auto write_smart_object_layer = [&](
		LayeredFile<bpp_type> document, 
		std::shared_ptr<SmartObjectLayer<bpp_type>> layer, 
		std::filesystem::path _path,
		std::filesystem::path _reference_path)
		{
			auto channels = layer->get_image_data();
			auto width = layer->width();
			auto height = layer->height();
			auto offset_x = static_cast<int>(std::round(layer->center_x()));
			auto offset_y = static_cast<int>(std::round(layer->center_y()));

			auto channel_r = channels.at(0);
			auto channel_g = channels.at(1);
			auto channel_b = channels.at(2);
			auto channel_a = channels.at(-1);

			auto channel_r_buffer = Render::ConstChannelBuffer<bpp_type>(channel_r, width, height, offset_x, offset_y);
			auto channel_g_buffer = Render::ConstChannelBuffer<bpp_type>(channel_g, width, height, offset_x, offset_y);
			auto channel_b_buffer = Render::ConstChannelBuffer<bpp_type>(channel_b, width, height, offset_x, offset_y);
			auto channel_a_buffer = Render::ConstChannelBuffer<bpp_type>(channel_a, width, height, offset_x, offset_y);

			auto image = Render::ConstImageBuffer<bpp_type>({ 
				{0, channel_r_buffer},
				{1, channel_g_buffer},
				{2, channel_b_buffer},
				{-1, channel_a_buffer}
				}, layer->name(), std::nullopt, Geometry::Point2D<int>(offset_x, offset_y));

			auto canvas_r_buffer = std::vector<bpp_type>(document.width() * document.height());
			auto canvas_g_buffer = std::vector<bpp_type>(document.width() * document.height());
			auto canvas_b_buffer = std::vector<bpp_type>(document.width() * document.height());

			auto canvas = Render::ImageBuffer<bpp_type>({
					{0, Render::ChannelBuffer<bpp_type>(canvas_r_buffer, document.width(), document.height()) },
					{1, Render::ChannelBuffer<bpp_type>(canvas_g_buffer, document.width(), document.height()) },
					{2, Render::ChannelBuffer<bpp_type>(canvas_b_buffer, document.width(), document.height()) },
				});

			Composite::composite_rgb<bpp_type, float>(canvas, image, Enum::BlendMode::Normal);
			// Write for debugging purposes
			canvas.write(_path);

			compare_layer(_reference_path, canvas);
		};



	std::filesystem::create_directories(base_out_path);
	std::filesystem::create_directories(base_out_path / "simple_warp");
	std::filesystem::create_directories(base_out_path / "quilt_warp");

	for (const auto& layer : file.layers())
	{
		if (auto smart_object_layer = dynamic_pointer_cast<SmartObjectLayer<bpp_type>>(layer))
		{
			auto path = base_out_path / fmt::format("{}.png", smart_object_layer->name());
			auto ref_path = base_ref_path / fmt::format("{}.png", smart_object_layer->name());
			write_smart_object_layer(file, smart_object_layer, path, ref_path);
		}
	}
	for (const auto& layer : find_layer_as<bpp_type, GroupLayer>("simple_warp", file)->layers())
	{
		if (auto smart_object_layer = dynamic_pointer_cast<SmartObjectLayer<bpp_type>>(layer))
		{
			auto path = base_out_path / "simple_warp" / fmt::format("{}.png", smart_object_layer->name());
			auto ref_path = base_ref_path / "simple_warp" / fmt::format("{}.png", smart_object_layer->name());
			write_smart_object_layer(file, smart_object_layer, path, ref_path);
		}
	}
	for (const auto& layer : find_layer_as<bpp_type, GroupLayer>("quilt_warp", file)->layers())
	{
		if (auto smart_object_layer = dynamic_pointer_cast<SmartObjectLayer<bpp_type>>(layer))
		{
			auto path = base_out_path / "quilt_warp" / fmt::format("{}.png", smart_object_layer->name());
			auto ref_path = base_ref_path / "quilt_warp" / fmt::format("{}.png", smart_object_layer->name());
			write_smart_object_layer(file, smart_object_layer, path, ref_path);
		}
	}
}



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Roundtrip layer read-write internal linkage")
{
	using namespace NAMESPACE_PSAPI;
	using bpp_type = uint8_t;

	auto file = LayeredFile<bpp_type>(Enum::ColorMode::RGB, 64, 64);

	Layer<bpp_type>::Params lr_params{};
	lr_params.name = "SmartObject";
	lr_params.width = 64;
	lr_params.height = 32;

	auto layer = std::make_shared<SmartObjectLayer<bpp_type>>(file, lr_params, "documents/image_data/ImageStackerImage.jpg");
	file.add_layer(layer);

	LayeredFile<bpp_type>::write(std::move(file), "smart_object_out.psd");

	auto read_file = LayeredFile<bpp_type>::read("smart_object_out.psd");
	auto read_layer = find_layer_as<bpp_type, SmartObjectLayer>("SmartObject", read_file);

	auto image_data = read_layer->get_image_data();
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Roundtrip layer read-write external linkage")
{
	using namespace NAMESPACE_PSAPI;
	using bpp_type = uint8_t;

	auto file = LayeredFile<bpp_type>(Enum::ColorMode::RGB, 64, 64);

	Layer<bpp_type>::Params lr_params{};
	lr_params.name = "SmartObject";
	lr_params.width = 64;
	lr_params.height = 32;

	auto layer = std::make_shared<SmartObjectLayer<bpp_type>>(file, lr_params, "documents/image_data/ImageStackerImage.jpg", LinkedLayerType::external);
	file.add_layer(layer);

	LayeredFile<bpp_type>::write(std::move(file), "smart_object_out.psd");

	auto read_file = LayeredFile<bpp_type>::read("smart_object_out.psd");
	auto read_layer = find_layer_as<bpp_type, SmartObjectLayer>("SmartObject", read_file);

	auto image_data = read_layer->get_image_data();
}



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Roundtrip layer read-write mixed linkage")
{
	using namespace NAMESPACE_PSAPI;
	using bpp_type = uint8_t;

	auto file = LayeredFile<bpp_type>(Enum::ColorMode::RGB, 64, 64);

	Layer<bpp_type>::Params lr_params{};
	lr_params.name = "SmartObject";
	Layer<bpp_type>::Params lr_params2{};
	lr_params2.name = "SmartObject2";

	auto layer = std::make_shared<SmartObjectLayer<bpp_type>>(file, lr_params, "documents/image_data/ImageStackerImage.jpg", LinkedLayerType::external);
	file.add_layer(layer);
	auto layer_external = std::make_shared<SmartObjectLayer<bpp_type>>(file, lr_params2, "documents/image_data/uv_grid.jpg");
	file.add_layer(layer_external);

	LayeredFile<bpp_type>::write(std::move(file), "smart_object_out.psd");

	auto read_file = LayeredFile<bpp_type>::read("smart_object_out.psd");
	auto read_layer = find_layer_as<bpp_type, SmartObjectLayer>("SmartObject", read_file);
	auto read_layer2 = find_layer_as<bpp_type, SmartObjectLayer>("SmartObject2", read_file);

	auto image_data = read_layer->get_image_data();
	auto image_data2 = read_layer2->get_image_data();
}



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#ifndef ARM_MAC_ARCH
TEST_CASE("Create layer invalid filepath"
	* doctest::no_breaks(true)
	* doctest::no_output(true)
	* doctest::should_fail(true))
{
	using namespace NAMESPACE_PSAPI;
	using bpp_type = uint8_t;

	auto file = LayeredFile<bpp_type>(Enum::ColorMode::RGB, 64, 64);

	Layer<bpp_type>::Params lr_params{};
	lr_params.name = "SmartObject";

	auto layer = std::make_shared<SmartObjectLayer<bpp_type>>(file, lr_params, "foo/bar.jpg", LinkedLayerType::external);
}
#endif