#include "doctest.h"

#include "Macros.h"
#include "Compression/RLE.h"
#include "PhotoshopFile/PhotoshopFile.h"

#include <vector>
#include <filesystem>


// This tests the sample provided in the wikipedia page for the packbits algorithm and this is the exact implementation used
// by photoshop as well
TEST_CASE("Test Wikipedia Packbits Example")
{
	// Equates to 'FE AA 02 80 00 2A FD AA 03 80 00 2A 22 F7 AA' in hexadecimal
	std::vector<uint8_t> data = { 254u, 170u, 2u, 128u, 0u, 42u, 253u, 170u, 3u, 128u, 0u, 42u, 34u, 247u, 170u };
	std::vector<uint8_t> expected = { 170u, 170u, 170u, 128u, 0u, 42u, 170u, 170u, 170u, 170u, 128u, 0u, 42u, 34u, 170u, 170u, 170u, 170u, 170u, 170u, 170u, 170u, 170u, 170u };

	SUBCASE("Defining no width and height")
	{
		// Note that we do not need to actually provide a width and height here as that only allows for premature reserving of the data
		// but is not strictly necessary
		std::vector<uint8_t> resultNoSize = NAMESPACE_PSAPI::DecompressPackBits<uint8_t>(data, 0u, 0u);
		CHECK(resultNoSize == expected);
	}

	SUBCASE("Defining too large width and height")
	{
		std::vector<uint8_t> resultTooLarge = NAMESPACE_PSAPI::DecompressPackBits<uint8_t>(data, 128u, 128u);
		CHECK(resultTooLarge == expected);
	}
}


TEST_CASE("Decompress file with RLE compression")
{

	// This document is 64x64 pixels, 8 bit and its channels are all compressed using RLE
	// There are 5 layers in total which each represent different types of data
	//		- "LayerRed":			Layer that is entirely red, we expect the red channel to be entirely white (255) while the rest is 0
	//		- "LayerGreen":			Same as above but entirely green
	//		- "LayerBlue":			Same as above but entirely blue
	//		- "LayerFirstRowRed":	The entire layer is black except for the first row which is red (255, 0, 0). We expect the data to reflect this
	//		- "Layer_R255_G128_B0":	The layer has the R, G and B values indicated in the layer name across the whole document
	std::filesystem::path combined_path = std::filesystem::current_path();
	combined_path += "\\documents\\Compression\\Compression_RLE_8bit.psd";


	NAMESPACE_PSAPI::File file(combined_path);
	NAMESPACE_PSAPI::PhotoshopFile document;
	bool didParse = document.read(file);

	NAMESPACE_PSAPI::LayerInfo& layerInformation = document.m_LayerMaskInfo.m_LayerInfo;

	SUBCASE("Check Layer Count is read correctly")
	{
		std::size_t layerRecordLength = layerInformation.m_LayerRecords.size();
		std::size_t channelImageDataLength = layerInformation.m_ChannelImageData.size();

		REQUIRE(layerRecordLength == 5);
		REQUIRE(channelImageDataLength == 5);
	}

	SUBCASE("Check 'LayerRed'")
	{
		int layerIndex = layerInformation.getLayerIndex("LayerRed");

		SUBCASE("Could find layer")
		{
			CHECK(layerIndex != -1);
		}

		SUBCASE("Channels are correct")
		{
			
			NAMESPACE_PSAPI::ChannelImageData& channelImageData = layerInformation.m_ChannelImageData.at(layerIndex);
			
			int channel_r_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Red);
			int channel_g_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Green);
			int channel_b_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Blue);
			int channel_a_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::TransparencyMask);

			CHECK(channel_r_index != -1);
			CHECK(channel_g_index != -1);
			CHECK(channel_b_index != -1);
			CHECK(channel_a_index != -1);

			std::vector<uint8_t> expected_r(64 * 64, 255);
			std::vector<uint8_t> expected_bg(64 * 64, 0);

			// We could also extract directly using this signature and skip the step above
			// channelImageData.extractImageData<uint8_t>(NAMESPACE_PSAPI::Enum::ChannelID::Red)
			CHECK(channelImageData.extractImageData<uint8_t>(channel_r_index) == expected_r);
			CHECK(channelImageData.extractImageData<uint8_t>(channel_g_index) == expected_bg);
			CHECK(channelImageData.extractImageData<uint8_t>(channel_b_index) == expected_bg);
			// Alpha channel is white
			CHECK(channelImageData.extractImageData<uint8_t>(channel_a_index) == expected_r);

		}
	}

	SUBCASE("Check 'LayerFirstRowRed'")
	{
		int layerIndex = layerInformation.getLayerIndex("LayerFirstRowRed");

		SUBCASE("Could find layer")
		{
			CHECK(layerIndex != -1);
		}

		SUBCASE("Channels are correct")
		{
			NAMESPACE_PSAPI::ChannelImageData& channelImageData = layerInformation.m_ChannelImageData.at(layerIndex);

			int channel_r_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Red);
			int channel_g_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Green);
			int channel_b_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Blue);
			int channel_a_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::TransparencyMask);

			CHECK(channel_r_index != -1);
			CHECK(channel_g_index != -1);
			CHECK(channel_b_index != -1);
			CHECK(channel_a_index != -1);

			// Fill the first row with white values
			std::vector<uint8_t> expected_r(64 * 64, 0);
			for (int i = 0; i < 64; ++i)
			{
				expected_r[i] = 255;
			}
			std::vector<uint8_t> expected_bg(64 * 64, 0);
			std::vector<uint8_t> expected_a(64 * 64, 255);

			CHECK(channelImageData.extractImageData<uint8_t>(channel_r_index) == expected_r);
			CHECK(channelImageData.extractImageData<uint8_t>(channel_g_index) == expected_bg);
			CHECK(channelImageData.extractImageData<uint8_t>(channel_b_index) == expected_bg);
			// Alpha channel is white
			CHECK(channelImageData.extractImageData<uint8_t>(channel_a_index) == expected_a);
		}
	}


	SUBCASE("Check 'Layer_R255_G128_B0'")
	{
		int layerIndex = layerInformation.getLayerIndex("Layer_R255_G128_B0");

		SUBCASE("Could find layer")
		{
			CHECK(layerIndex != -1);
		}

		SUBCASE("Channels are correct")
		{
			NAMESPACE_PSAPI::ChannelImageData& channelImageData = layerInformation.m_ChannelImageData.at(layerIndex);

			int channel_r_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Red);
			int channel_g_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Green);
			int channel_b_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Blue);
			int channel_a_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::TransparencyMask);

			CHECK(channel_r_index != -1);
			CHECK(channel_g_index != -1);
			CHECK(channel_b_index != -1);
			CHECK(channel_a_index != -1);

				
			std::vector<uint8_t> expected_r(64 * 64, 255);
			std::vector<uint8_t> expected_g(64 * 64, 128);
			std::vector<uint8_t> expected_b(64 * 64, 0);

			CHECK(channelImageData.extractImageData<uint8_t>(channel_r_index) == expected_r);
			CHECK(channelImageData.extractImageData<uint8_t>(channel_g_index) == expected_g);
			CHECK(channelImageData.extractImageData<uint8_t>(channel_b_index) == expected_b);
			// Alpha channel is white
			CHECK(channelImageData.extractImageData<uint8_t>(channel_a_index) == expected_r);
		}
	}

	CHECK(didParse);
}