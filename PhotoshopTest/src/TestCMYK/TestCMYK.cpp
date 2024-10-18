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
TEST_CASE("Create CMYK File 8-bit psd")
{
	using namespace NAMESPACE_PSAPI;
	using type = bpp8_t;
	constexpr type value = 255u;
	constexpr int32_t width = 64;
	constexpr int32_t height = 64;
	constexpr int32_t size = width * height;
	constexpr Enum::ColorMode colormode = Enum::ColorMode::CMYK;


	auto file = LayeredFile<type>(colormode, width, height);
	auto params = Layer<type>::Params{
		.layerName = "Layer",
		.width = width,
		.height = height,
		.colorMode = colormode,
	};
	std::unordered_map<int16_t, std::vector<type>> img_data = {
		{0, std::vector<type>(width * height, value)},
		{1, std::vector<type>(width * height, value)},
		{2, std::vector<type>(width * height, value)},
		{3, std::vector<type>(width * height, value)},
		{-1, std::vector<type>(width * height, value)},
	};
	auto img_layer = std::make_shared<ImageLayer<type>>(std::move(img_data), params);
	file.addLayer(img_layer);
	params = Layer<type>::Params{
		.layerName = "Group",
		.colorMode = colormode,
	};
	auto grp_layer = std::make_shared<GroupLayer<type>>(params);
	file.addLayer(grp_layer);
	LayeredFile<type>::write(std::move(file), "CMYK.psd");
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Create CMYK File 8-bit psb")
{
	using namespace NAMESPACE_PSAPI;
	using type = bpp8_t;
	constexpr type value = 255u;
	constexpr int32_t width = 64;
	constexpr int32_t height = 64;
	constexpr int32_t size = width * height;
	constexpr Enum::ColorMode colormode = Enum::ColorMode::CMYK;


	auto file = LayeredFile<type>(colormode, width, height);
	auto params = Layer<type>::Params{
		.layerName = "Layer",
		.width = width,
		.height = height,
		.colorMode = colormode,
	};
	std::unordered_map<int16_t, std::vector<type>> img_data = {
		{0, std::vector<type>(width * height, value)},
		{1, std::vector<type>(width * height, value)},
		{2, std::vector<type>(width * height, value)},
		{3, std::vector<type>(width * height, value)},
		{-1, std::vector<type>(width * height, value)},
	};
	auto img_layer = std::make_shared<ImageLayer<type>>(std::move(img_data), params);
	file.addLayer(img_layer);
	params = Layer<type>::Params{
		.layerName = "Group",
		.colorMode = colormode,
	};
	auto grp_layer = std::make_shared<GroupLayer<type>>(params);
	file.addLayer(grp_layer);
	LayeredFile<type>::write(std::move(file), "CMYK.psb");
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Create CMYK File 16-bit psd")
{
	using namespace NAMESPACE_PSAPI;
	using type = bpp16_t;
	constexpr type value = 65535u;
	constexpr int32_t width = 64;
	constexpr int32_t height = 64;
	constexpr int32_t size = width * height;
	constexpr Enum::ColorMode colormode = Enum::ColorMode::CMYK;


	auto file = LayeredFile<type>(colormode, width, height);
	auto params = Layer<type>::Params{
		.layerName = "Layer",
		.width = width,
		.height = height,
		.colorMode = colormode,
	};
	std::unordered_map<int16_t, std::vector<type>> img_data = {
		{0, std::vector<type>(width * height, value)},
		{1, std::vector<type>(width * height, value)},
		{2, std::vector<type>(width * height, value)},
		{3, std::vector<type>(width * height, value)},
		{-1, std::vector<type>(width * height, value)},
	};
	auto img_layer = std::make_shared<ImageLayer<type>>(std::move(img_data), params);
	file.addLayer(img_layer);
	params = Layer<type>::Params{
		.layerName = "Group",
		.colorMode = colormode,
	};
	auto grp_layer = std::make_shared<GroupLayer<type>>(params);
	file.addLayer(grp_layer);
	LayeredFile<type>::write(std::move(file), "CMYK.psd");
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Create CMYK File 16-bit psb")
{
	using namespace NAMESPACE_PSAPI;
	using type = bpp16_t;
	constexpr type value = 65535u;
	constexpr int32_t width = 64;
	constexpr int32_t height = 64;
	constexpr int32_t size = width * height;
	constexpr Enum::ColorMode colormode = Enum::ColorMode::CMYK;


	auto file = LayeredFile<type>(colormode, width, height);
	auto params = Layer<type>::Params{
		.layerName = "Layer",
		.width = width,
		.height = height,
		.colorMode = colormode,
	};
	std::unordered_map<int16_t, std::vector<type>> img_data = {
		{0, std::vector<type>(width * height, value)},
		{1, std::vector<type>(width * height, value)},
		{2, std::vector<type>(width * height, value)},
		{3, std::vector<type>(width * height, value)},
		{-1, std::vector<type>(width * height, value)},
	};
	auto img_layer = std::make_shared<ImageLayer<type>>(std::move(img_data), params);
	file.addLayer(img_layer);
	params = Layer<type>::Params{
		.layerName = "Group",
		.colorMode = colormode,
	};
	auto grp_layer = std::make_shared<GroupLayer<type>>(params);
	file.addLayer(grp_layer);
	LayeredFile<type>::write(std::move(file), "CMYK.psd");
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#ifndef ARM_MAC_ARCH
TEST_CASE("Create CMYK File 32-bit"
	* doctest::no_breaks(true)
	* doctest::no_output(true)
	* doctest::should_fail(true))
{
	using namespace NAMESPACE_PSAPI;
	using type = bpp32_t;
	constexpr type value = 65535u;
	constexpr int32_t width = 64;
	constexpr int32_t height = 64;
	constexpr int32_t size = width * height;
	constexpr Enum::ColorMode colormode = Enum::ColorMode::CMYK;


	auto file = LayeredFile<type>(colormode, width, height);
	auto params = Layer<type>::Params{
		.layerName = "Layer",
		.width = width,
		.height = height,
		.colorMode = colormode,
	};
	std::unordered_map<int16_t, std::vector<type>> img_data = {
		{0, std::vector<type>(width * height, value)},
		{1, std::vector<type>(width * height, value)},
		{2, std::vector<type>(width * height, value)},
		{3, std::vector<type>(width * height, value)},
		{-1, std::vector<type>(width * height, value)},
	};
	auto img_layer = std::make_shared<ImageLayer<type>>(std::move(img_data), params);
	file.addLayer(img_layer);
	params = Layer<type>::Params{
		.layerName = "Group",
		.colorMode = colormode,
	};
	auto grp_layer = std::make_shared<GroupLayer<type>>(params);
	file.addLayer(grp_layer);
	LayeredFile<type>::write(std::move(file), "CMYK.psd");
}
#endif