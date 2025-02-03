#include "doctest.h"

#include "../DetectArmMac.h"

#include "Macros.h"
#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/ImageLayer.h"
#include "LayeredFile/LayerTypes/GroupLayer.h"

#include <string>
#include <vector>



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Create Grayscale File 8-bit psd")
{
	using namespace NAMESPACE_PSAPI;
	using type = bpp8_t;
	constexpr type value = 255u;
	constexpr int32_t width = 64;
	constexpr int32_t height = 64;
	constexpr int32_t size = width * height;
	constexpr Enum::ColorMode colormode = Enum::ColorMode::Grayscale;


	auto file = LayeredFile<type>(colormode, width, height);
	auto params = Layer<type>::Params{
		.name = "Layer",
		.width = width,
		.height = height,
		.colormode = colormode,
	};
	std::unordered_map<int, std::vector<type>> img_data = {
		{0, std::vector<type>(width * height, value)},
		{-1, std::vector<type>(width * height, value)},
	};
	auto img_layer = std::make_shared<ImageLayer<type>>(std::move(img_data), params);
	file.add_layer(img_layer);
	params = Layer<type>::Params{
		.name = "Group",
		.colormode = colormode,
	};
	auto grp_layer = std::make_shared<GroupLayer<type>>(params);
	file.add_layer(grp_layer);
	LayeredFile<type>::write(std::move(file), "Grayscale.psd");
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Create Grayscale File 8-bit psb")
{
	using namespace NAMESPACE_PSAPI;
	using type = bpp8_t;
	constexpr type value = 255u;
	constexpr int32_t width = 64;
	constexpr int32_t height = 64;
	constexpr int32_t size = width * height;
	constexpr Enum::ColorMode colormode = Enum::ColorMode::Grayscale;


	auto file = LayeredFile<type>(colormode, width, height);
	auto params = Layer<type>::Params{
		.name = "Layer",
		.width = width,
		.height = height,
		.colormode = colormode,
	};
	std::unordered_map<int, std::vector<type>> img_data = {
		{0, std::vector<type>(width * height, value)},
		{-1, std::vector<type>(width * height, value)},
	};
	auto img_layer = std::make_shared<ImageLayer<type>>(std::move(img_data), params);
	file.add_layer(img_layer);
	params = Layer<type>::Params{
		.name = "Group",
		.colormode = colormode,
	};
	auto grp_layer = std::make_shared<GroupLayer<type>>(params);
	file.add_layer(grp_layer);
	LayeredFile<type>::write(std::move(file), "Grayscale.psb");
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Create Grayscale File 16-bit psd")
{
	using namespace NAMESPACE_PSAPI;
	using type = bpp16_t;
	constexpr type value = 65535u;
	constexpr int32_t width = 64;
	constexpr int32_t height = 64;
	constexpr int32_t size = width * height;
	constexpr Enum::ColorMode colormode = Enum::ColorMode::Grayscale;


	auto file = LayeredFile<type>(colormode, width, height);
	auto params = Layer<type>::Params{
		.name = "Layer",
		.width = width,
		.height = height,
		.colormode = colormode,
	};
	std::unordered_map<int, std::vector<type>> img_data = {
		{0, std::vector<type>(width * height, value)},
		{-1, std::vector<type>(width * height, value)},
	};
	auto img_layer = std::make_shared<ImageLayer<type>>(std::move(img_data), params);
	file.add_layer(img_layer);
	params = Layer<type>::Params{
		.name = "Group",
		.colormode = colormode,
	};
	auto grp_layer = std::make_shared<GroupLayer<type>>(params);
	file.add_layer(grp_layer);
	LayeredFile<type>::write(std::move(file), "Grayscale.psd");
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Create Grayscale File 16-bit psb")
{
	using namespace NAMESPACE_PSAPI;
	using type = bpp16_t;
	constexpr type value = 65535u;
	constexpr int32_t width = 64;
	constexpr int32_t height = 64;
	constexpr int32_t size = width * height;
	constexpr Enum::ColorMode colormode = Enum::ColorMode::Grayscale;


	auto file = LayeredFile<type>(colormode, width, height);
	auto params = Layer<type>::Params{
		.name = "Layer",
		.width = width,
		.height = height,
		.colormode = colormode,
	};
	std::unordered_map<int, std::vector<type>> img_data = {
		{0, std::vector<type>(width * height, value)},
		{-1, std::vector<type>(width * height, value)},
	};
	auto img_layer = std::make_shared<ImageLayer<type>>(std::move(img_data), params);
	file.add_layer(img_layer);
	params = Layer<type>::Params{
		.name = "Group",
		.colormode = colormode,
	};
	auto grp_layer = std::make_shared<GroupLayer<type>>(params);
	file.add_layer(grp_layer);
	LayeredFile<type>::write(std::move(file), "Grayscale.psd");
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Create Grayscale File 32-bit psd")
{
	using namespace NAMESPACE_PSAPI;
	using type = bpp32_t;
	constexpr type value = 1.0f;
	constexpr int32_t width = 64;
	constexpr int32_t height = 64;
	constexpr int32_t size = width * height;
	constexpr Enum::ColorMode colormode = Enum::ColorMode::Grayscale;


	auto file = LayeredFile<type>(colormode, width, height);
	auto params = Layer<type>::Params{
		.name = "Layer",
		.width = width,
		.height = height,
		.colormode = colormode,
	};
	std::unordered_map<int, std::vector<type>> img_data = {
		{0, std::vector<type>(width * height, value)},
		{-1, std::vector<type>(width * height, value)},
	};
	auto img_layer = std::make_shared<ImageLayer<type>>(std::move(img_data), params);
	file.add_layer(img_layer);
	params = Layer<type>::Params{
		.name = "Group",
		.colormode = colormode,
	};
	auto grp_layer = std::make_shared<GroupLayer<type>>(params);
	file.add_layer(grp_layer);
	LayeredFile<type>::write(std::move(file), "Grayscale.psd");
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Create Grayscale File 32-bit psb")
{
	using namespace NAMESPACE_PSAPI;
	using type = bpp32_t;
	constexpr type value = 1.0f;
	constexpr int32_t width = 64;
	constexpr int32_t height = 64;
	constexpr int32_t size = width * height;
	constexpr Enum::ColorMode colormode = Enum::ColorMode::Grayscale;


	auto file = LayeredFile<type>(colormode, width, height);
	auto params = Layer<type>::Params{
		.name = "Layer",
		.width = width,
		.height = height,
		.colormode = colormode,
	};
	std::unordered_map<int, std::vector<type>> img_data = {
		{0, std::vector<type>(width * height, value)},
		{-1, std::vector<type>(width * height, value)},
	};
	auto img_layer = std::make_shared<ImageLayer<type>>(std::move(img_data), params);
	file.add_layer(img_layer);
	params = Layer<type>::Params{
		.name = "Group",
		.colormode = colormode,
	};
	auto grp_layer = std::make_shared<GroupLayer<type>>(params);
	file.add_layer(grp_layer);
	LayeredFile<type>::write(std::move(file), "Grayscale.psd");
}
