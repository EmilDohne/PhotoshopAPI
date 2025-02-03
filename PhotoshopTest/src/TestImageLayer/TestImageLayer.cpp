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


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Construct ImageLayer with mask as part of image data")
{
	using namespace NAMESPACE_PSAPI;
	using type = bpp16_t;
	constexpr int32_t width = 64;
	constexpr int32_t height = 64;
	constexpr int32_t size = width * height;

	std::unordered_map<int, std::vector<type>> data =
	{
		{-2, std::vector<type>(size)},
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
	CHECK(layer->has_mask());
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

	std::unordered_map<int, std::vector<type>> data =
	{
		{0, std::vector<type>(size)},
		{1, std::vector<type>(size)},
		{2, std::vector<type>(size)},
	};
	auto params = typename Layer<type>::Params
	{
		.mask = std::vector<type>(size),
		.name = "Layer",
		.width = width,
		.height = height,
	};

	auto layer = std::make_shared<ImageLayer<type>>(data, params);

	CHECK(layer->width() == width);
	CHECK(layer->height() == height);
	CHECK(layer->name() == "Layer");
	CHECK(layer->has_mask());
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

	std::unordered_map<int, std::vector<type>> data =
	{
		{-2, std::vector<type>(size)},
		{0, std::vector<type>(size)},
		{1, std::vector<type>(size)},
		{2, std::vector<type>(size)},
	};
	auto params = typename Layer<type>::Params
	{
		.mask = std::vector<type>(size),
		.name = "Layer",
		.width = width,
		.height = height,
	};

	auto layer = std::make_shared<ImageLayer<type>>(data, params);

	CHECK(layer->width() == width);
	CHECK(layer->height() == height);
	CHECK(layer->name() == "Layer");
	CHECK(layer->has_mask());
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

	std::unordered_map<int, std::vector<type>> data =
	{
		{-2, std::vector<type>(size)},
		{0, std::vector<type>(size)},
		{1, std::vector<type>(size)},
		{3, std::vector<type>(size)},
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
	CHECK(layer->has_mask());
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

	std::unordered_map<int, std::vector<type>> data =
	{
		{-2, std::vector<type>(size)},
		{0, std::vector<type>(size)},
		{1, std::vector<type>(size)},
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
	CHECK(layer->has_mask());
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

	std::unordered_map<int, std::vector<type>> data =
	{
		{-2, std::vector<type>(size)},
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
	CHECK(layer->has_mask());

	std::vector<type> channel(size, 65535u);
	layer->set_channel(Enum::ChannelID::Red, channel);
	CHECK(layer->get_channel(Enum::ChannelID::Red) == channel);
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

	std::unordered_map<int, std::vector<type>> data =
	{
		{-2, std::vector<type>(size)},
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
	CHECK(layer->has_mask());

	std::vector<type> channel(size, 65535u);
	layer->set_channel(2, channel);
	CHECK(layer->get_channel(2) == channel);
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

	std::vector<type> channel(size, 65535u);
	layer->set_channel(-2, channel);
	CHECK(layer->get_channel(-2) == channel);
	CHECK(layer->has_mask());
	CHECK(layer->get_mask() == channel);
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

	std::unordered_map<int, std::vector<type>> data =
	{
		{-2, std::vector<type>(size)},
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
	CHECK(layer->has_mask());

	std::vector<type> channel(size, 65535u);
	layer->set_channel(Enum::ChannelID::Cyan, channel);
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

	std::unordered_map<int, std::vector<type>> data =
	{
		{-2, std::vector<type>(size)},
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
	CHECK(layer->has_mask());

	std::vector<type> channel(size - 5u, 65535u);
	layer->set_channel(Enum::ChannelID::Red, channel);
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

	std::unordered_map<int, std::vector<type>> data =
	{
		{-2, std::vector<type>(size)},
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
	CHECK(layer->has_mask());


	std::vector<type> channel(size, 65535u);
	std::unordered_map<Enum::ChannelID, std::vector<type>> data_new =
	{
		{Enum::ChannelID::UserSuppliedLayerMask, std::vector<type>(size)},
		{Enum::ChannelID::Red, channel},
		{Enum::ChannelID::Green, std::vector<type>(size)},
		{Enum::ChannelID::Blue, std::vector<type>(size)},
	};
	layer->set_image_data(std::move(data_new));
	CHECK(layer->has_mask());
	CHECK(layer->get_channel(Enum::ChannelID::Red) == channel);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Set layer data with int")
{
	using namespace NAMESPACE_PSAPI;
	using type = bpp16_t;
	constexpr int32_t width = 64;
	constexpr int32_t height = 64;
	constexpr int32_t size = width * height;

	std::unordered_map<int, std::vector<type>> data =
	{
		{-2, std::vector<type>(size)},
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
	CHECK(layer->has_mask());

	std::vector<type> channel(size, 65535u);
	std::unordered_map<int, std::vector<type>> data_new =
	{
		{-2, std::vector<type>(size)},
		{0, channel},
		{1, std::vector<type>(size)},
		{2, std::vector<type>(size)},
	};
	layer->set_image_data(std::move(data_new));
	CHECK(layer->has_mask());
	CHECK(layer->get_channel(Enum::ChannelID::Red) == channel);
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

	std::unordered_map<int, std::vector<type>> data =
	{
		{-2, std::vector<type>(size)},
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
	CHECK(layer->has_mask());

	std::vector<type> channel(size, 65535u);
	std::unordered_map<int, std::vector<type>> data_new =
	{
		{-2, std::vector<type>(size)},
		{0, channel},
		{1, std::vector<type>(size)},
		{2, std::vector<type>(size)},
		{3, std::vector<type>(size)},
	};
	layer->set_image_data(std::move(data_new));
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

	std::unordered_map<int, std::vector<type>> data =
	{
		{-2, std::vector<type>(size)},
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
	CHECK(layer->has_mask());

	std::vector<type> channel(size, 65535u);
	std::unordered_map<int, std::vector<type>> data_new =
	{
		{-2, std::vector<type>(size)},
		{0, channel},
		{1, std::vector<type>(size + 5)},
		{2, std::vector<type>(size)},
	};
	layer->set_image_data(std::move(data_new));
}
#endif