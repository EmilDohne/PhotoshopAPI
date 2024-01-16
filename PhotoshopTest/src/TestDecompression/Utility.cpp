#include "Utility.h"

#include "Macros.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "Struct/TaggedBlock.h"
#include "Enum.h"
#include "../TestMacros.h"

#include <vector>
#include <filesystem>
#include <type_traits>
#include <optional>

/// Explicitly instantiate these templates
template void checkDecompressionFileImpl<uint8_t>(NAMESPACE_PSAPI::LayerInfo& layerInformation, const double zero_val, const double val_128, const double one_val, const double red_zero_val);
template void checkDecompressionFileImpl<uint16_t>(NAMESPACE_PSAPI::LayerInfo& layerInformation, const double zero_val, const double val_128, const double one_val, const double red_zero_val);
template void checkDecompressionFileImpl<float32_t>(NAMESPACE_PSAPI::LayerInfo& layerInformation, const double zero_val, const double val_128, const double one_val, const double red_zero_val);


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
void checkDecompressionFileImpl(NAMESPACE_PSAPI::LayerInfo& layerInformation, const double zero_val, const double val_128, const double one_val, const double red_zero_val)
{
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
		REQUIRE(layerIndex != -1);

		NAMESPACE_PSAPI::ChannelImageData& channelImageData = layerInformation.m_ChannelImageData.at(layerIndex);

		int channel_r_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Red);
		int channel_g_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Green);
		int channel_b_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Blue);
		int channel_a_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Alpha);

		REQUIRE(channel_r_index != -1);
		REQUIRE(channel_g_index != -1);
		REQUIRE(channel_b_index != -1);
		REQUIRE(channel_a_index != -1);

		// We could also extract directly using this signature and skip the step above
		// channelImageData.extractImageData<T>(NAMESPACE_PSAPI::Enum::ChannelID::Red)
		std::vector<T> channel_r = channelImageData.extractImageData<T>(channel_r_index);
		std::vector<T> channel_g = channelImageData.extractImageData<T>(channel_g_index);
		std::vector<T> channel_b = channelImageData.extractImageData<T>(channel_b_index);
		std::vector<T> channel_a = channelImageData.extractImageData<T>(channel_a_index);

		std::vector<T> expected_r(64 * 64, one_val);
		std::vector<T> expected_bg(64 * 64, red_zero_val);

			
		CHECK_VEC_ALMOST_EQUAL(channel_r, expected_r);
		CHECK_VEC_ALMOST_EQUAL(channel_b, expected_bg);
		CHECK_VEC_ALMOST_EQUAL(channel_g, expected_bg);
		// Alpha channel is white
		CHECK_VEC_ALMOST_EQUAL(channel_a, expected_r);
	}

	SUBCASE("Check 'LayerFirstRowRed'")
	{
		int layerIndex = layerInformation.getLayerIndex("LayerFirstRowRed");
		REQUIRE(layerIndex != -1);

		NAMESPACE_PSAPI::ChannelImageData& channelImageData = layerInformation.m_ChannelImageData.at(layerIndex);

		int channel_r_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Red);
		int channel_g_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Green);
		int channel_b_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Blue);
		int channel_a_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Alpha);

		REQUIRE(channel_r_index != -1);
		REQUIRE(channel_g_index != -1);
		REQUIRE(channel_b_index != -1);
		REQUIRE(channel_a_index != -1);

		// We could also extract directly using this signature and skip the step above
		// channelImageData.extractImageData<T>(NAMESPACE_PSAPI::Enum::ChannelID::Red)
		std::vector<T> channel_r = channelImageData.extractImageData<T>(channel_r_index);
		std::vector<T> channel_g = channelImageData.extractImageData<T>(channel_g_index);
		std::vector<T> channel_b = channelImageData.extractImageData<T>(channel_b_index);
		std::vector<T> channel_a = channelImageData.extractImageData<T>(channel_a_index);

		// Fill the first row with white values
		std::vector<T> expected_r(64 * 64, zero_val);
		for (int i = 0; i < 64; ++i)
		{
			expected_r[i] = one_val;
		}
		std::vector<T> expected_bg(64 * 64, zero_val);
		for (int i = 0; i < 64; ++i)
		{
			expected_bg[i] = red_zero_val;
		}
		std::vector<T> expected_a(64 * 64, one_val);

		CHECK_VEC_ALMOST_EQUAL(channel_r, expected_r);
		CHECK_VEC_ALMOST_EQUAL(channel_b, expected_bg);
		CHECK_VEC_ALMOST_EQUAL(channel_g, expected_bg);
		// Alpha channel is white
		CHECK_VEC_ALMOST_EQUAL(channel_a, expected_a);
	}

	SUBCASE("Check 'Layer_R255_G128_B0'")
	{
		int layerIndex = layerInformation.getLayerIndex("Layer_R255_G128_B0");
		REQUIRE(layerIndex != -1);

		NAMESPACE_PSAPI::ChannelImageData& channelImageData = layerInformation.m_ChannelImageData.at(layerIndex);

		int channel_r_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Red);
		int channel_g_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Green);
		int channel_b_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Blue);
		int channel_a_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Alpha);

		// We could also extract directly using this signature and skip the step above
		// channelImageData.extractImageData<T>(NAMESPACE_PSAPI::Enum::ChannelID::Red)
		std::vector<T> channel_r = channelImageData.extractImageData<T>(channel_r_index);
		std::vector<T> channel_g = channelImageData.extractImageData<T>(channel_g_index);
		std::vector<T> channel_b = channelImageData.extractImageData<T>(channel_b_index);
		std::vector<T> channel_a = channelImageData.extractImageData<T>(channel_a_index);

		REQUIRE(channel_r_index != -1);
		REQUIRE(channel_g_index != -1);
		REQUIRE(channel_b_index != -1);
		REQUIRE(channel_a_index != -1);

		std::vector<T> expected_r(64 * 64, one_val);
		std::vector<T> expected_g(64 * 64, val_128);
		std::vector<T> expected_b(64 * 64, zero_val);


		CHECK_VEC_ALMOST_EQUAL(channel_r, expected_r);
		CHECK_VEC_ALMOST_EQUAL(channel_g, expected_g);
		CHECK_VEC_ALMOST_EQUAL(channel_b, expected_b);
		// Alpha channel is white
		CHECK_VEC_ALMOST_EQUAL(channel_a, expected_r);
	}

}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
void checkDecompressionFile(std::filesystem::path& inputPath, const double zero_val, const double val_128, const double one_val, const double red_zero_val)
{
	PSAPI_LOG_ERROR("CheckCompressionFile", "Unimplemented template type");
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
void checkDecompressionFile<uint8_t>(std::filesystem::path& inputPath, const double zero_val, const double val_128, const double one_val, const double red_zero_val)
{
	NAMESPACE_PSAPI::File file(inputPath);
	NAMESPACE_PSAPI::PhotoshopFile document;
	document.read(file);

	// 8-bit file store their layerInfo normally
	NAMESPACE_PSAPI::LayerInfo& layerInformation = document.m_LayerMaskInfo.m_LayerInfo;
	checkDecompressionFileImpl<uint8_t>(layerInformation, zero_val, val_128, one_val, red_zero_val);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
void checkDecompressionFile<uint16_t>(std::filesystem::path& inputPath, const double zero_val, const double val_128, const double one_val, const double red_zero_val)
{
	NAMESPACE_PSAPI::File file(inputPath);
	NAMESPACE_PSAPI::PhotoshopFile document;
	document.read(file);

	// 16-bit files store their layerInformation in the additional tagged blocks
	REQUIRE(document.m_LayerMaskInfo.m_AdditionalLayerInfo.has_value());
	const auto& additionalLayerInfo = document.m_LayerMaskInfo.m_AdditionalLayerInfo.value();
	auto lr16TaggedBlock = additionalLayerInfo.getTaggedBlock<NAMESPACE_PSAPI::Lr16TaggedBlock>(NAMESPACE_PSAPI::Enum::TaggedBlockKey::Lr16);
	REQUIRE(lr16TaggedBlock.has_value());
	NAMESPACE_PSAPI::LayerInfo& layerInformation = lr16TaggedBlock.value()->m_Data;

	checkDecompressionFileImpl<uint16_t>(layerInformation, zero_val, val_128, one_val, red_zero_val);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
void checkDecompressionFile<float32_t>(std::filesystem::path& inputPath, const double zero_val, const double val_128, const double one_val, const double red_zero_val)
{
	NAMESPACE_PSAPI::File file(inputPath);
	NAMESPACE_PSAPI::PhotoshopFile document;
	document.read(file);

	// 16-bit files store their layerInformation in the additional tagged blocks
	REQUIRE(document.m_LayerMaskInfo.m_AdditionalLayerInfo.has_value());
	const auto& additionalLayerInfo = document.m_LayerMaskInfo.m_AdditionalLayerInfo.value();
	auto lr32TaggedBlock = additionalLayerInfo.getTaggedBlock<NAMESPACE_PSAPI::Lr32TaggedBlock>(NAMESPACE_PSAPI::Enum::TaggedBlockKey::Lr32);
	REQUIRE(lr32TaggedBlock.has_value());
	NAMESPACE_PSAPI::LayerInfo& layerInformation = lr32TaggedBlock.value()->m_Data;

	checkDecompressionFileImpl<float32_t>(layerInformation, zero_val, val_128, one_val, red_zero_val);
}