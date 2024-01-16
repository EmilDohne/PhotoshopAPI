#include "Utility.h"

#include "Macros.h"
#include "FileIO/Read.h"
#include "FileIO/Write.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "Struct/TaggedBlock.h"
#include "Enum.h"

#include <vector>
#include <tuple>
#include <filesystem>

// Explicitly instantiate these templates
template void checkCompressionFileImpl<uint8_t>(NAMESPACE_PSAPI::LayerInfo& layerInformation, const NAMESPACE_PSAPI::FileHeader& header, NAMESPACE_PSAPI::File& document);
template void checkCompressionFileImpl<uint16_t>(NAMESPACE_PSAPI::LayerInfo& layerInformation, const NAMESPACE_PSAPI::FileHeader& header, NAMESPACE_PSAPI::File& document);
template void checkCompressionFileImpl<float32_t>(NAMESPACE_PSAPI::LayerInfo& layerInformation, const NAMESPACE_PSAPI::FileHeader& header, NAMESPACE_PSAPI::File& document);



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
void checkLayerIsSame(NAMESPACE_PSAPI::LayerInfo& layerInformation, const NAMESPACE_PSAPI::FileHeader& header, NAMESPACE_PSAPI::File& document, int layerIndex)
{
	NAMESPACE_PSAPI::ChannelImageData& channelImageData = layerInformation.m_ChannelImageData.at(layerIndex);
	uint32_t width = layerInformation.m_LayerRecords.at(layerIndex).getWidth();
	uint32_t height = layerInformation.m_LayerRecords.at(layerIndex).getHeight();

	int channel_r_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Red);
	int channel_g_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Green);
	int channel_b_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Blue);
	int channel_a_index = channelImageData.getChannelIndex(NAMESPACE_PSAPI::Enum::ChannelID::Alpha);

	REQUIRE(channel_r_index != -1);
	REQUIRE(channel_g_index != -1);
	REQUIRE(channel_b_index != -1);
	REQUIRE(channel_a_index != -1);

	// Offset these by 2 bytes to get the actual compressed data
	auto offsetsAndSizes = channelImageData.getChannelOffsetsAndSizes();
	std::vector<uint8_t> documentDataR;
	std::vector<uint8_t> documentDataG;
	std::vector<uint8_t> documentDataB;
	std::vector<uint8_t> documentDataA;
	{
		uint64_t offset = std::get<0>(offsetsAndSizes.at(channel_r_index)) + 2u;
		uint64_t size = std::get<1>(offsetsAndSizes.at(channel_r_index)) - 2u;
		documentDataR = NAMESPACE_PSAPI::ReadBinaryArray<uint8_t>(document, offset, size);
	}
	{
		uint64_t offset = std::get<0>(offsetsAndSizes.at(channel_g_index)) + 2u;
		uint64_t size = std::get<1>(offsetsAndSizes.at(channel_g_index)) - 2u;
		documentDataG = NAMESPACE_PSAPI::ReadBinaryArray<uint8_t>(document, offset, size);
	}
	{
		uint64_t offset = std::get<0>(offsetsAndSizes.at(channel_b_index)) + 2u;
		uint64_t size = std::get<1>(offsetsAndSizes.at(channel_b_index)) - 2u;
		documentDataB = NAMESPACE_PSAPI::ReadBinaryArray<uint8_t>(document, offset, size);
	}
	{
		uint64_t offset = std::get<0>(offsetsAndSizes.at(channel_a_index)) + 2u;
		uint64_t size = std::get<1>(offsetsAndSizes.at(channel_a_index)) - 2u;
		documentDataA = NAMESPACE_PSAPI::ReadBinaryArray<uint8_t>(document, offset, size);
	}

	std::vector<uint8_t> compressedDataR;
	std::vector<uint8_t> compressedDataG;
	std::vector<uint8_t> compressedDataB;
	std::vector<uint8_t> compressedDataA;
	{
		auto channelData = channelImageData.extractImageData<T>(channel_r_index);
		const auto channelCompression = channelImageData.getChannelCompression(channel_r_index);
		compressedDataR = NAMESPACE_PSAPI::CompressData<T>(channelData, channelCompression, header, width, height);
	}
	{
		auto channelData = channelImageData.extractImageData<T>(channel_g_index);
		const auto channelCompression = channelImageData.getChannelCompression(channel_g_index);
		compressedDataG = NAMESPACE_PSAPI::CompressData<T>(channelData, channelCompression, header, width, height);
	}
	{
		auto channelData = channelImageData.extractImageData<T>(channel_b_index);
		const auto channelCompression = channelImageData.getChannelCompression(channel_b_index);
		compressedDataB = NAMESPACE_PSAPI::CompressData<T>(channelData, channelCompression, header, width, height);
	}
	{
		auto channelData = channelImageData.extractImageData<T>(channel_a_index);
		const auto channelCompression = channelImageData.getChannelCompression(channel_a_index);
		compressedDataA = NAMESPACE_PSAPI::CompressData<T>(channelData, channelCompression, header, width, height);
	}

	CHECK(compressedDataR == documentDataR);
	CHECK(compressedDataG == documentDataG);
	CHECK(compressedDataB == documentDataB);
	CHECK(compressedDataA == documentDataA);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
void checkCompressionFileImpl(NAMESPACE_PSAPI::LayerInfo& layerInformation, const NAMESPACE_PSAPI::FileHeader& header, NAMESPACE_PSAPI::File& document)
{
	// For the roundtripping we actually need to grab the offsets for each of the channels to just read the binary data 
	// so that we have a baseline to compare against

	SUBCASE("Check 'LayerRed'")
	{
		int layerIndex = layerInformation.getLayerIndex("LayerRed");
		REQUIRE(layerIndex != -1);
		checkLayerIsSame<T>(layerInformation, header, document, layerIndex);
	}
	SUBCASE("Check 'LayerFirstRowRed'")
	{
		int layerIndex = layerInformation.getLayerIndex("LayerFirstRowRed");
		REQUIRE(layerIndex != -1);
		checkLayerIsSame<T>(layerInformation, header, document, layerIndex);
	}
	SUBCASE("Check 'Layer_R255_G128_B0'")
	{
		int layerIndex = layerInformation.getLayerIndex("Layer_R255_G128_B0");
		REQUIRE(layerIndex != -1);
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
void checkCompressionFile(std::filesystem::path& inputPath)
{
	PSAPI_LOG_ERROR("CheckCompressionFile", "Unimplemented template type");
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
void checkCompressionFile<uint8_t>(std::filesystem::path& inputPath)
{
	NAMESPACE_PSAPI::File file(inputPath);
	NAMESPACE_PSAPI::PhotoshopFile document;
	document.read(file);

	// 8-bit file store their layerInfo normally
	NAMESPACE_PSAPI::LayerInfo& layerInformation = document.m_LayerMaskInfo.m_LayerInfo;
	checkCompressionFileImpl<uint8_t>(layerInformation, document.m_Header, file);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
void checkCompressionFile<uint16_t>(std::filesystem::path& inputPath)
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

	checkCompressionFileImpl<uint16_t>(layerInformation, document.m_Header, file);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template<>
void checkCompressionFile<float32_t>(std::filesystem::path& inputPath)
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

	checkCompressionFileImpl<float32_t>(layerInformation, document.m_Header, file);
}