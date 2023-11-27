#include "Utility.h"

#include "Macros.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "Struct/TaggedBlock.h"
#include "Enum.h"

#include <vector>
#include <filesystem>
#include <type_traits>
#include <optional>

// Explicitly instantiate these templates
template void checkCompressionFile<uint8_t>(std::filesystem::path& inputPath, const double zero_val, const double val_128, const double one_val);
template void checkCompressionFile<uint16_t>(std::filesystem::path& inputPath, const double zero_val, const double val_128, const double one_val);
template void checkCompressionFile<float32_t>(std::filesystem::path& inputPath, const double zero_val, const double val_128, const double one_val);

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
void checkCompressionFile(std::filesystem::path& inputPath, const double zero_val, const double val_128, const double one_val)
{
	// This document is 64x64 pixels, 8 bit and its channels are all compressed using RLE
	// There are 5 layers in total which each represent different types of data
	//		- "LayerRed":			Layer that is entirely red, we expect the red channel to be entirely white (255) while the rest is 0
	//		- "LayerGreen":			Same as above but entirely green
	//		- "LayerBlue":			Same as above but entirely blue
	//		- "LayerFirstRowRed":	The entire layer is black except for the first row which is red (255, 0, 0). We expect the data to reflect this
	//		- "Layer_R255_G128_B0":	The layer has the R, G and B values indicated in the layer name across the whole document

	NAMESPACE_PSAPI::File file(inputPath);
	NAMESPACE_PSAPI::PhotoshopFile document;
	bool didParse = document.read(file);

	// 16 and 32 bit files store their layerInformation in the additional tagged blocks
	NAMESPACE_PSAPI::LayerInfo& layerInformation = document.m_LayerMaskInfo.m_LayerInfo;;
	if (sizeof(T) == 1)
	{
		// No need to reassign
	}
	else if (sizeof(T) == 2)
	{
		REQUIRE(document.m_LayerMaskInfo.m_AdditionalLayerInfo.has_value());
		auto& additionalLayerInfo = document.m_LayerMaskInfo.m_AdditionalLayerInfo.value();
		auto lr16TaggedBlock = additionalLayerInfo.getTaggedBlock<NAMESPACE_PSAPI::TaggedBlock::Lr16>(NAMESPACE_PSAPI::Enum::TaggedBlockKey::Lr16);
		REQUIRE(lr16TaggedBlock.has_value());
		layerInformation = lr16TaggedBlock.value()->m_Data;
	}
	else if (sizeof(T) == 4)
	{
		REQUIRE(document.m_LayerMaskInfo.m_AdditionalLayerInfo.has_value());
		auto& additionalLayerInfo = document.m_LayerMaskInfo.m_AdditionalLayerInfo.value();
		auto lr32TaggedBlock = additionalLayerInfo.getTaggedBlock<NAMESPACE_PSAPI::TaggedBlock::Lr32>(NAMESPACE_PSAPI::Enum::TaggedBlockKey::Lr32);
		REQUIRE(lr32TaggedBlock.has_value());
		layerInformation = lr32TaggedBlock.value()->m_Data;
	}
	else
	{
		REQUIRE(false);
	}

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

			std::vector<T> expected_r(64 * 64, one_val);
			std::vector<T> expected_bg(64 * 64, zero_val);

			// We could also extract directly using this signature and skip the step above
			// channelImageData.extractImageData<T>(NAMESPACE_PSAPI::Enum::ChannelID::Red)
			CHECK(channelImageData.extractImageData<T>(channel_r_index) == expected_r);
			CHECK(channelImageData.extractImageData<T>(channel_g_index) == expected_bg);
			CHECK(channelImageData.extractImageData<T>(channel_b_index) == expected_bg);
			// Alpha channel is white
			CHECK(channelImageData.extractImageData<T>(channel_a_index) == expected_r);

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
			std::vector<T> expected_r(64 * 64, zero_val);
			for (int i = 0; i < 64; ++i)
			{
				expected_r[i] = one_val;
			}
			std::vector<T> expected_bg(64 * 64, zero_val);
			std::vector<T> expected_a(64 * 64, one_val);

			CHECK(channelImageData.extractImageData<T>(channel_r_index) == expected_r);
			CHECK(channelImageData.extractImageData<T>(channel_g_index) == expected_bg);
			CHECK(channelImageData.extractImageData<T>(channel_b_index) == expected_bg);
			// Alpha channel is white
			CHECK(channelImageData.extractImageData<T>(channel_a_index) == expected_a);
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

			std::vector<T> expected_r(64 * 64, one_val);
			std::vector<T> expected_g(64 * 64, val_128);
			std::vector<T> expected_b(64 * 64, zero_val);


			CHECK(channelImageData.extractImageData<T>(channel_r_index) == expected_r);
			CHECK(channelImageData.extractImageData<T>(channel_g_index) == expected_g);
			CHECK(channelImageData.extractImageData<T>(channel_b_index) == expected_b);
			// Alpha channel is white
			CHECK(channelImageData.extractImageData<T>(channel_a_index) == expected_r);
		}
	}

	CHECK(didParse);
}