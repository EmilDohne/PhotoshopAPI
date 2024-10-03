#include "doctest.h"

#include "../DetectArmMac.h"

#include "Macros.h"
#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/ImageLayer.h"

#include <string>
#include <vector>



TEST_CASE("Construct ImageLayer with int16_t ctor")
{
	using namespace NAMESPACE_PSAPI;
	using type = bpp16_t;
	constexpr int32_t width = 64;
	constexpr int32_t height = 64;
	constexpr int32_t size = width * height;

	std::unordered_map<int16_t, std::vector<type>> data =
	{
		{0, std::vector<type>(size)},
		{1, std::vector<type>(size)},
		{2, std::vector<type>(size)},
	};
	auto params = typename Layer<type>::Params
	{
		.layerName = "Layer",
		.width = width,
		.height = height,
	};

	auto layer = std::make_shared<ImageLayer<type>>(std::move(data), params);

	CHECK(layer->m_Width == width);
	CHECK(layer->m_Height == height);
	CHECK(layer->m_LayerName == "Layer");
	CHECK(layer->m_ImageData.size() == data.size());
}


TEST_CASE("Construct ImageLayer with mask as part of image data")
{
	using namespace NAMESPACE_PSAPI;
	using type = bpp16_t;
	constexpr int32_t width = 64;
	constexpr int32_t height = 64;
	constexpr int32_t size = width * height;

	std::unordered_map<int16_t, std::vector<type>> data =
	{
		{-2, std::vector<type>(size)},
		{0, std::vector<type>(size)},
		{1, std::vector<type>(size)},
		{2, std::vector<type>(size)},
	};
	auto params = typename Layer<type>::Params
	{
		.layerName = "Layer",
		.width = width,
		.height = height,
	};

	auto layer = std::make_shared<ImageLayer<type>>(std::move(data), params);

	CHECK(layer->m_Width == width);
	CHECK(layer->m_Height == height);
	CHECK(layer->m_LayerName == "Layer");
	CHECK(layer->m_LayerMask);
}


TEST_CASE("Construct ImageLayer with explicit mask")
{
	using namespace NAMESPACE_PSAPI;
	using type = bpp16_t;
	constexpr int32_t width = 64;
	constexpr int32_t height = 64;
	constexpr int32_t size = width * height;

	std::unordered_map<int16_t, std::vector<type>> data =
	{
		{0, std::vector<type>(size)},
		{1, std::vector<type>(size)},
		{2, std::vector<type>(size)},
	};
	auto params = typename Layer<type>::Params
	{
		.layerMask = std::vector<type>(size),
		.layerName = "Layer",
		.width = width,
		.height = height,
	};

	auto layer = std::make_shared<ImageLayer<type>>(std::move(data), params);

	CHECK(layer->m_Width == width);
	CHECK(layer->m_Height == height);
	CHECK(layer->m_LayerName == "Layer");
	CHECK(layer->m_LayerMask);
}


#ifndef ARM_MAC_ARCH
TEST_CASE("Construct ImageLayer with both mask as part of imagedata and through layer parameters"
	* doctest::no_breaks(true)
	* doctest::no_output(true)
	* doctest::should_fail(true))
{
	using namespace NAMESPACE_PSAPI;
	using type = bpp16_t;
	constexpr int32_t width = 64;
	constexpr int32_t height = 64;
	constexpr int32_t size = width * height;

	std::unordered_map<int16_t, std::vector<type>> data =
	{
		{-2, std::vector<type>(size)},
		{0, std::vector<type>(size)},
		{1, std::vector<type>(size)},
		{2, std::vector<type>(size)},
	};
	auto params = typename Layer<type>::Params
	{
		.layerMask = std::vector<type>(size),
		.layerName = "Layer",
		.width = width,
		.height = height,
	};

	auto layer = std::make_shared<ImageLayer<type>>(std::move(data), params);

	CHECK(layer->m_Width == width);
	CHECK(layer->m_Height == height);
	CHECK(layer->m_LayerName == "Layer");
	CHECK(layer->m_LayerMask);
}
#endif
