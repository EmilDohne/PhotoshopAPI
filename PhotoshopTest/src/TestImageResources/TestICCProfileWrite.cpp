#include "doctest.h"

#include "PhotoshopFile/PhotoshopFile.h"
#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/ImageLayer.h"
#include "LayeredFile/LayerTypes/Layer.h"
#include "Macros.h"

#include <filesystem>



TEST_CASE("Write AdobeRGB1998")
{
	using namespace NAMESPACE_PSAPI;

	std::filesystem::path psb_path = std::filesystem::current_path();
	psb_path += "/documents/ICCProfiles/Write_AdobeRGB1998.psb";

	std::filesystem::path icc_path = std::filesystem::current_path();
	icc_path += "/documents/ICCProfiles/AdobeRGB1998.icc";

	// Write out the file with the given ICC Profile
	{
		const uint32_t width = 64u;
		const uint32_t height = 64u;
		LayeredFile<bpp8_t> document = { Enum::ColorMode::RGB, width, height };
		document.icc_profile(ICCProfile(icc_path));

		std::unordered_map <Enum::ChannelID, std::vector<bpp8_t>> channelMap;
		channelMap[Enum::ChannelID::Red] = std::vector<bpp8_t>(width * height, 36u);
		channelMap[Enum::ChannelID::Green] = std::vector<bpp8_t>(width * height, 36u);
		channelMap[Enum::ChannelID::Blue] = std::vector<bpp8_t>(width * height, 36u);

		ImageLayer<bpp8_t>::Params layerParams = {};
		layerParams.name = "Layer";
		layerParams.width = width;
		layerParams.height = height;

		auto layer = std::make_shared<ImageLayer<bpp8_t>>(
			std::move(channelMap),
			layerParams
		);
		document.add_layer(layer);

		File::FileParams params = File::FileParams();
		params.doRead = false;
		params.forceOverwrite = true;
		auto outputFile = File(psb_path, params);
		auto psdDocumentPtr = layered_to_photoshop(std::move(document), psb_path);
		ProgressCallback callback{};
		psdDocumentPtr->write(outputFile, callback);
	}

	// Read it back in and check if we actually have the right profile
	{
		auto inputFile = File(psb_path);
		auto psDocumentPtr = std::make_unique<PhotoshopFile>();
		ProgressCallback callback{};
		psDocumentPtr->read(inputFile, callback);
		LayeredFile<bpp8_t> layeredFile = { std::move(psDocumentPtr), psb_path};

		// Get the ICC Profile we read from the PSB
		std::vector<uint8_t> readICCProfile = layeredFile.icc_profile().data();

		// Get the ICC profile directly from disk
		File iccFile = { icc_path };
		std::vector<uint8_t> diskICCData = ReadBinaryArray<uint8_t>(iccFile, iccFile.getSize());

		CHECK(readICCProfile == diskICCData);
	}
}


TEST_CASE("Write AppleRGB")
{
	using namespace NAMESPACE_PSAPI;

	std::filesystem::path psb_path = std::filesystem::current_path();
	psb_path += "/documents/ICCProfiles/Write_AppleRGB.psb";

	std::filesystem::path icc_path = std::filesystem::current_path();
	icc_path += "/documents/ICCProfiles/AppleRGB.icc";

	// Write out the file with the given ICC Profile
	{
		const uint32_t width = 64u;
		const uint32_t height = 64u;
		LayeredFile<bpp8_t> document = { Enum::ColorMode::RGB, width, height };
		document.icc_profile(ICCProfile(icc_path));

		std::unordered_map <Enum::ChannelID, std::vector<bpp8_t>> channelMap;
		channelMap[Enum::ChannelID::Red] = std::vector<bpp8_t>(width * height, 36u);
		channelMap[Enum::ChannelID::Green] = std::vector<bpp8_t>(width * height, 36u);
		channelMap[Enum::ChannelID::Blue] = std::vector<bpp8_t>(width * height, 36u);

		ImageLayer<bpp8_t>::Params layerParams = {};
		layerParams.name = "Layer";
		layerParams.width = width;
		layerParams.height = height;

		auto layer = std::make_shared<ImageLayer<bpp8_t>>(
			std::move(channelMap),
			layerParams
		);
		document.add_layer(layer);

		File::FileParams params = File::FileParams();
		params.doRead = false;
		params.forceOverwrite = true;
		auto outputFile = File(psb_path, params);
		auto psdDocumentPtr = layered_to_photoshop(std::move(document), psb_path);
		ProgressCallback callback{};
		psdDocumentPtr->write(outputFile, callback);
	}

	// Read it back in and check if we actually have the right profile
	{
		auto inputFile = File(psb_path);
		auto psDocumentPtr = std::make_unique<PhotoshopFile>();
		ProgressCallback callback{};
		psDocumentPtr->read(inputFile, callback);
		LayeredFile<bpp8_t> layeredFile = { std::move(psDocumentPtr), psb_path };

		// Get the ICC Profile we read from the PSB
		std::vector<uint8_t> readICCProfile = layeredFile.icc_profile().data();

		// Get the ICC profile directly from disk
		File iccFile = { icc_path };
		std::vector<uint8_t> diskICCData = ReadBinaryArray<uint8_t>(iccFile, iccFile.getSize());

		CHECK(readICCProfile == diskICCData);
	}
}


TEST_CASE("Write CIERGB")
{
	using namespace NAMESPACE_PSAPI;

	std::filesystem::path psb_path = std::filesystem::current_path();
	psb_path += "/documents/ICCProfiles/Write_CIERGB.psb";

	std::filesystem::path icc_path = std::filesystem::current_path();
	icc_path += "/documents/ICCProfiles/CIERGB.icc";

	// Write out the file with the given ICC Profile
	{
		const uint32_t width = 64u;
		const uint32_t height = 64u;
		LayeredFile<bpp8_t> document = { Enum::ColorMode::RGB, width, height };
		document.icc_profile(ICCProfile(icc_path));

		std::unordered_map <Enum::ChannelID, std::vector<bpp8_t>> channelMap;
		channelMap[Enum::ChannelID::Red] = std::vector<bpp8_t>(width * height, 36u);
		channelMap[Enum::ChannelID::Green] = std::vector<bpp8_t>(width * height, 36u);
		channelMap[Enum::ChannelID::Blue] = std::vector<bpp8_t>(width * height, 36u);

		ImageLayer<bpp8_t>::Params layerParams = {};
		layerParams.name = "Layer";
		layerParams.width = width;
		layerParams.height = height;

		auto layer = std::make_shared<ImageLayer<bpp8_t>>(
			std::move(channelMap),
			layerParams
		);
		document.add_layer(layer);

		File::FileParams params = File::FileParams();
		params.doRead = false;
		params.forceOverwrite = true;
		auto outputFile = File(psb_path, params);
		auto psdDocumentPtr = layered_to_photoshop(std::move(document), psb_path);
		ProgressCallback callback{};
		psdDocumentPtr->write(outputFile, callback);
	}

	// Read it back in and check if we actually have the right profile
	{
		auto inputFile = File(psb_path);
		auto psDocumentPtr = std::make_unique<PhotoshopFile>();
		ProgressCallback callback{};
		psDocumentPtr->read(inputFile, callback);
		LayeredFile<bpp8_t> layeredFile = { std::move(psDocumentPtr), psb_path };

		// Get the ICC Profile we read from the PSB
		std::vector<uint8_t> readICCProfile = layeredFile.icc_profile().data();

		// Get the ICC profile directly from disk
		File iccFile = { icc_path };
		std::vector<uint8_t> diskICCData = ReadBinaryArray<uint8_t>(iccFile, iccFile.getSize());

		CHECK(readICCProfile == diskICCData);
	}
}