#include "doctest.h"

#include "../DetectArmMac.h"

#include "Macros.h"
#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/ImageLayer.h"

#include <string>
#include <vector>



TEST_CASE("Retrieve a single channel")
{
	using namespace NAMESPACE_PSAPI;

	LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read("documents/Compression/Compression_RLE_8bit.psb");
	auto imageLayerPtr = find_layer_as<bpp8_t, ImageLayer>("Layer_R255_G128_B0", layeredFile);

	std::vector<bpp8_t> channel_r = imageLayerPtr->get_channel(Enum::ChannelID::Red);
	std::vector<bpp8_t> channel_g = imageLayerPtr->get_channel(Enum::ChannelID::Green);
	std::vector<bpp8_t> channel_b = imageLayerPtr->get_channel(Enum::ChannelID::Blue);

	std::vector<bpp8_t> expected_r(layeredFile.width() * layeredFile.height(), 255u);
	std::vector<bpp8_t> expected_g(layeredFile.width() * layeredFile.height(), 128u);
	std::vector<bpp8_t> expected_b(layeredFile.width() * layeredFile.height(), 0u);

	CHECK(channel_r == expected_r);
	CHECK(channel_g == expected_g);
	CHECK(channel_b == expected_b);
}



TEST_CASE("Retrieve all channels")
{
	using namespace NAMESPACE_PSAPI;

	LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read("documents/Compression/Compression_RLE_8bit.psb");
	auto imageLayerPtr = find_layer_as<bpp8_t, ImageLayer>("Layer_R255_G128_B0", layeredFile);

	auto channels = imageLayerPtr->get_image_data();

	std::vector<bpp8_t> expected_r(layeredFile.width() * layeredFile.height(), 255u);
	std::vector<bpp8_t> expected_g(layeredFile.width() * layeredFile.height(), 128u);
	std::vector<bpp8_t> expected_b(layeredFile.width() * layeredFile.height(), 0u);

	for (auto& [key, value] : channels)
	{
		if (key.id == Enum::ChannelID::Red)
		{
			CHECK(value == expected_r);
		}
		else if (key.id == Enum::ChannelID::Green)
		{
			CHECK(value == expected_g);
		}
		else if (key.id == Enum::ChannelID::Blue)
		{
			CHECK(value == expected_b);
		}
	}
}


TEST_CASE("Double extract data")
{
	using namespace NAMESPACE_PSAPI;

	LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read("documents/Compression/Compression_RLE_8bit.psb");
	auto imageLayerPtr = find_layer_as<bpp8_t, ImageLayer>("Layer_R255_G128_B0", layeredFile);
	

	std::vector<bpp8_t> expected_r(layeredFile.width() * layeredFile.height(), 255u);
	std::vector<bpp8_t> expected_g(layeredFile.width() * layeredFile.height(), 128u);
	std::vector<bpp8_t> expected_b(layeredFile.width() * layeredFile.height(), 0u);

	// First extraction, doCopy is defaulted to true so we should be able to extract again after
	{
		auto channels = imageLayerPtr->get_image_data();
		for (auto& [key, value] : channels)
		{
			if (key.id == Enum::ChannelID::Red)
			{
				CHECK(value == expected_r);
			}
			else if (key.id == Enum::ChannelID::Green)
			{
				CHECK(value == expected_g);
			}
			else if (key.id == Enum::ChannelID::Blue)
			{
				CHECK(value == expected_b);
			}
		}
	}

	// Second extraction, doCopy is defaulted to true so we should be able to extract
	{
		auto channels = imageLayerPtr->get_image_data();
		for (auto& [key, value] : channels)
		{
			if (key.id == Enum::ChannelID::Red)
			{
				CHECK(value == expected_r);
			}
			else if (key.id == Enum::ChannelID::Green)
			{
				CHECK(value == expected_g);
			}
			else if (key.id == Enum::ChannelID::Blue)
			{
				CHECK(value == expected_b);
			}
		}
	}
}


TEST_CASE("Double extract channel")
{
	using namespace NAMESPACE_PSAPI;

	LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read("documents/Compression/Compression_RLE_8bit.psb");
	auto imageLayerPtr = find_layer_as<bpp8_t, ImageLayer>("Layer_R255_G128_B0", layeredFile);

	// doCopy is defaulted to true so we should not be extracting data
	std::vector<bpp8_t> channel_g = imageLayerPtr->get_channel(Enum::ChannelID::Green);
	std::vector<bpp8_t> channel_g_2 = imageLayerPtr->get_channel(Enum::ChannelID::Green);
	std::vector<bpp8_t> expected_g(layeredFile.width() * layeredFile.height(), 128u);

	CHECK(channel_g == expected_g);
	CHECK(channel_g_2 == expected_g);
}


#ifndef ARM_MAC_ARCH
	TEST_CASE("Double extract channel without copy"
		* doctest::no_breaks(true)
		* doctest::no_output(true)
		* doctest::should_fail(true))
	{
		using namespace NAMESPACE_PSAPI;

		LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read("documents/Compression/Compression_RLE_8bit.psb");
		auto imageLayerPtr = find_layer_as<bpp8_t, ImageLayer>("Layer_R255_G128_B0", layeredFile);

		// This is expected to fail
		std::vector<bpp8_t> channel_g = imageLayerPtr->get_channel(Enum::ChannelID::Green, false);
		std::vector<bpp8_t> channel_g_2 = imageLayerPtr->get_channel(Enum::ChannelID::Green, false);
	}
#endif

#ifndef ARM_MAC_ARCH
	TEST_CASE("Double extract all channels without copy"
		* doctest::no_breaks(true)
		* doctest::no_output(true)
		* doctest::should_fail(true))
	{
		using namespace NAMESPACE_PSAPI;

		LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read("documents/Compression/Compression_RLE_8bit.psb");
		auto imageLayerPtr = find_layer_as<bpp8_t, ImageLayer>("Layer_R255_G128_B0", layeredFile);

		// This is expected to fail
		auto channels = imageLayerPtr->get_image_data(false);
		auto channels2 = imageLayerPtr->get_image_data(false);
	}
#endif



TEST_CASE("Extract mask channel from group")
{
	using namespace NAMESPACE_PSAPI;

	LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read("documents/Masks/SingleMask_White.psb");
	auto groupLayerPtr = find_layer_as<bpp8_t, GroupLayer>("MaskGroup", layeredFile);
	auto imageLayerPtr = find_layer_as<bpp8_t, ImageLayer>("MaskGroup/MaskLayer", layeredFile);

	std::vector<bpp8_t> groupMaskChannel = groupLayerPtr->get_mask_data();
	std::vector<bpp8_t> imageMaskChannel = imageLayerPtr->get_mask_data();
	// Photoshop internally optimizes these mask channels which is why we have half the height 
	std::vector<bpp8_t> expectedMask(layeredFile.width() * layeredFile.height() / 2, 0u);

	CHECK(groupMaskChannel == expectedMask);
	CHECK(imageMaskChannel == expectedMask);
}


#ifndef ARM_MAC_ARCH
	TEST_CASE("Double extract mask channel from group without copy"
		* doctest::no_breaks(true)
		* doctest::no_output(true)
		* doctest::should_fail(true))
	{
		using namespace NAMESPACE_PSAPI;

		LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read("documents/Masks/SingleMask_White.psb");
		auto groupLayerPtr = find_layer_as<bpp8_t, GroupLayer>("MaskGroup", layeredFile);

		std::vector<bpp8_t> groupMaskChannel = groupLayerPtr->get_mask_data(false);
		std::vector<bpp8_t> groupMaskChannel2 = groupLayerPtr->get_mask_data(false);
	
	}
#endif


TEST_CASE("Double extract mask channel from group")
{
	using namespace NAMESPACE_PSAPI;

	LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read("documents/Masks/SingleMask_White.psb");
	auto groupLayerPtr = find_layer_as<bpp8_t, GroupLayer>("MaskGroup", layeredFile);

	std::vector<bpp8_t> groupMaskChannel = groupLayerPtr->get_mask_data();
	std::vector<bpp8_t> groupMaskChannel2 = groupLayerPtr->get_mask_data();
}