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


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
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


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
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


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
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


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#ifndef ARM_MAC_ARCH
TEST_CASE("Construct ImageLayer with invalid channels"
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
		{3, std::vector<type>(size)},
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
#endif


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#ifndef ARM_MAC_ARCH
TEST_CASE("Construct ImageLayer with too little channels"
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
#endif


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Set layer channel with Enum::ChannelID")
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

	std::vector<type> channel(size, 65535u);
	layer->setChannel(Enum::ChannelID::Red, channel);
	CHECK(layer->getChannel(Enum::ChannelID::Red, false) == channel);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Set layer channel with int16_t")
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

	std::vector<type> channel(size, 65535u);
	layer->setChannel(2, channel);
	CHECK(layer->getChannel(2, false) == channel);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Set layer channel mask channel with int16_t")
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

	std::vector<type> channel(size, 65535u);
	layer->setChannel(-2, channel);
	CHECK(layer->getChannel(-2, true) == channel);
	CHECK(layer->hasMask());
	CHECK(layer->getMask(true) == channel);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#ifndef ARM_MAC_ARCH
TEST_CASE("Set layer invalid channel"
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
		.layerName = "Layer",
		.width = width,
		.height = height,
	};

	auto layer = std::make_shared<ImageLayer<type>>(std::move(data), params);

	CHECK(layer->m_Width == width);
	CHECK(layer->m_Height == height);
	CHECK(layer->m_LayerName == "Layer");
	CHECK(layer->m_LayerMask);

	std::vector<type> channel(size, 65535u);
	layer->setChannel(Enum::ChannelID::Cyan, channel);
}
#endif


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#ifndef ARM_MAC_ARCH
TEST_CASE("Set layer invalid size channel"
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
		.layerName = "Layer",
		.width = width,
		.height = height,
	};

	auto layer = std::make_shared<ImageLayer<type>>(std::move(data), params);

	CHECK(layer->m_Width == width);
	CHECK(layer->m_Height == height);
	CHECK(layer->m_LayerName == "Layer");
	CHECK(layer->m_LayerMask);

	std::vector<type> channel(size - 5u, 65535u);
	layer->setChannel(Enum::ChannelID::Red, channel);
}
#endif



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Set layer data with Enum::ChannelID")
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


	std::vector<type> channel(size, 65535u);
	std::unordered_map<Enum::ChannelID, std::vector<type>> data_new =
	{
		{Enum::ChannelID::UserSuppliedLayerMask, std::vector<type>(size)},
		{Enum::ChannelID::Red, channel},
		{Enum::ChannelID::Green, std::vector<type>(size)},
		{Enum::ChannelID::Blue, std::vector<type>(size)},
	};
	layer->setImageData(std::move(data_new));
	CHECK(layer->hasMask());
	CHECK(layer->getChannel(Enum::ChannelID::Red, false) == channel);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Set layer data with int16_t")
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

	std::vector<type> channel(size, 65535u);
	std::unordered_map<int16_t, std::vector<type>> data_new =
	{
		{-2, std::vector<type>(size)},
		{0, channel},
		{1, std::vector<type>(size)},
		{2, std::vector<type>(size)},
	};
	layer->setImageData(std::move(data_new));
	CHECK(layer->hasMask());
	CHECK(layer->getChannel(Enum::ChannelID::Red, false) == channel);
}



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#ifndef ARM_MAC_ARCH
TEST_CASE("Set layer invalid channel"
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
		.layerName = "Layer",
		.width = width,
		.height = height,
	};

	auto layer = std::make_shared<ImageLayer<type>>(std::move(data), params);

	CHECK(layer->m_Width == width);
	CHECK(layer->m_Height == height);
	CHECK(layer->m_LayerName == "Layer");
	CHECK(layer->m_LayerMask);

	std::vector<type> channel(size, 65535u);
	std::unordered_map<int16_t, std::vector<type>> data_new =
	{
		{-2, std::vector<type>(size)},
		{0, channel},
		{1, std::vector<type>(size)},
		{2, std::vector<type>(size)},
		{3, std::vector<type>(size)},
	};
	layer->setImageData(std::move(data_new));
}
#endif


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#ifndef ARM_MAC_ARCH
TEST_CASE("Set layer invalid size channel"
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
		.layerName = "Layer",
		.width = width,
		.height = height,
	};

	auto layer = std::make_shared<ImageLayer<type>>(std::move(data), params);

	CHECK(layer->m_Width == width);
	CHECK(layer->m_Height == height);
	CHECK(layer->m_LayerName == "Layer");
	CHECK(layer->m_LayerMask);

	std::vector<type> channel(size, 65535u);
	std::unordered_map<int16_t, std::vector<type>> data_new =
	{
		{-2, std::vector<type>(size)},
		{0, channel},
		{1, std::vector<type>(size + 5)},
		{2, std::vector<type>(size)},
	};
	layer->setImageData(std::move(data_new));
}
#endif