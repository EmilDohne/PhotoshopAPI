#include "doctest.h"

#include "PhotoshopAPI.h"
#include "Core/Struct/DescriptorStructure.h"
#include "LayeredFile/LayerTypes/SmartObjectLayer.h"
#include "Core/Warp/SmartObjectWarp.h"

#include "Core/Render/ImageBuffer.h"
#include "Core/Render/Composite.h"
#include "Core/Render/Render.h"
#include "LayeredFile/LayeredFile.h"

#include <memory>
#include <filesystem>

#include <fmt/format.h>


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

	//CHECK(layer->width() == doctest::Approx(252.0f));
	//CHECK(layer->height() == doctest::Approx(161.0f));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Read all supported warps and write image files")
{
	using namespace NAMESPACE_PSAPI;
	using bpp_type = uint8_t;

	auto file = LayeredFile<bpp_type>::read(std::filesystem::current_path() / "documents/SmartObjects/smart_objects_transformed.psd");
	auto base_out_path = std::filesystem::current_path() / "documents/SmartObjects/out";

	auto write_smart_object_layer = [](LayeredFile<bpp_type> document, std::shared_ptr<SmartObjectLayer<bpp_type>> layer, std::filesystem::path _path)
		{
			auto channels = layer->get_image_data();
			auto width = layer->width();
			auto height = layer->height();
			auto offset_x = static_cast<int>(std::round(layer->center_x()));
			auto offset_y = static_cast<int>(std::round(layer->center_y()));
			auto channel_r = channels.at(Enum::toChannelIDInfo<int16_t>(0, Enum::ColorMode::RGB));
			auto channel_g = channels.at(Enum::toChannelIDInfo<int16_t>(1, Enum::ColorMode::RGB));
			auto channel_b = channels.at(Enum::toChannelIDInfo<int16_t>(2, Enum::ColorMode::RGB));
			auto channel_a = channels.at(Enum::toChannelIDInfo<int16_t>(-1, Enum::ColorMode::RGB));


			/*auto rendering_buffer = Render::ChannelBuffer<bpp_type>(channel_b, width, height, offset_x, offset_y);
			auto mesh = layer->warp().surface().mesh(20, 20, false);
			Render::render_mesh(rendering_buffer, mesh, static_cast<bpp_type>(255));*/

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

			/*auto canvas_r_buffer = std::vector<bpp_type>(document.width() * document.height());
			auto canvas_g_buffer = std::vector<bpp_type>(document.width() * document.height());
			auto canvas_b_buffer = std::vector<bpp_type>(document.width() * document.height());

			auto canvas = Render::ImageBuffer<bpp_type>({
					{0, Render::ChannelBuffer<bpp_type>(canvas_r_buffer, document.width(), document.height()) },
					{1, Render::ChannelBuffer<bpp_type>(canvas_g_buffer, document.width(), document.height()) },
					{2, Render::ChannelBuffer<bpp_type>(canvas_b_buffer, document.width(), document.height()) },
				});

			Composite::composite_rgb<bpp_type, float>(canvas, image, Enum::BlendMode::Normal);*/
			image.write(_path);
		};

	//std::filesystem::create_directories(base_out_path);
	std::filesystem::create_directories(base_out_path / "simple_warp");
	std::filesystem::create_directories(base_out_path / "quilt_warp");

	for (const auto& layer : file.layers())
	{
		if (auto smart_object_layer = dynamic_pointer_cast<SmartObjectLayer<bpp_type>>(layer))
		{
			write_smart_object_layer(file, smart_object_layer, base_out_path / fmt::format("{}.png", smart_object_layer->name()));
		}
	}
	for (const auto& layer : find_layer_as<bpp_type, GroupLayer>("simple_warp", file)->layers())
	{
		if (auto smart_object_layer = dynamic_pointer_cast<SmartObjectLayer<bpp_type>>(layer))
		{
			write_smart_object_layer(file, smart_object_layer, base_out_path / "simple_warp" / fmt::format("{}.png", smart_object_layer->name()));
		}
	}
	for (const auto& layer : find_layer_as<bpp_type, GroupLayer>("quilt_warp", file)->layers())
	{
		if (auto smart_object_layer = dynamic_pointer_cast<SmartObjectLayer<bpp_type>>(layer))
		{
			write_smart_object_layer(file, smart_object_layer, base_out_path / "quilt_warp" / fmt::format("{}.png", smart_object_layer->name()));
		}
	}
}