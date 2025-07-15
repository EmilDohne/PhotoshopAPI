#include "doctest.h"

#include "../DetectArmMac.h"

#include "Macros.h"
#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/ImageLayer.h"

#include <string>
#include <vector>


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Construct ImageLayer with int16_t ctor")
{
	using namespace NAMESPACE_PSAPI;
	using type = bpp16_t;
	constexpr int32_t width = 64;
	constexpr int32_t height = 64;
	constexpr int32_t size = width * height;

	std::unordered_map<int, std::vector<type>> data =
	{
		{0, std::vector<type>(size)},
		{1, std::vector<type>(size)},
		{2, std::vector<type>(size)},
	};
	auto params = typename Layer<type>::Params
	{
		.name = "Layer",
		.width = width,
		.height = height,
	};

	auto layer = std::make_shared<ImageLayer<type>>(data, params);

	CHECK(layer->width() == width);
	CHECK(layer->height() == height);
	CHECK(layer->name() == "Layer");
	CHECK(layer->num_channels(true) == data.size());
}