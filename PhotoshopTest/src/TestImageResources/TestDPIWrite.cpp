#include "doctest.h"

#include "PhotoshopFile/PhotoshopFile.h"
#include "LayeredFile/LayeredFile.h"
#include "Macros.h"

#include <filesystem>


TEST_CASE("Write DPI")
{
	// We first write, then read the DPI after which we compare. We must store an image layer as well as Photoshop requires at least one
	// imagelayer.

	using namespace NAMESPACE_PSAPI;

	std::filesystem::path psd_path = std::filesystem::current_path();
	psd_path += "/documents/DPI/Write_300dpi.psd";

	float dpi = 0.0f;
	

	// Write a file with a 300 dpi
	{
		const uint32_t width = 64u;
		const uint32_t height = 64u;
		LayeredFile<bpp8_t> document = { Enum::ColorMode::RGB, width, height };
		document.m_DotsPerInch = 300.0f;

		std::unordered_map <Enum::ChannelID, std::vector<bpp8_t>> channelMap;
		channelMap[Enum::ChannelID::Red] = std::vector<bpp8_t>(width * height, 36u);
		channelMap[Enum::ChannelID::Green] = std::vector<bpp8_t>(width * height, 36u);
		channelMap[Enum::ChannelID::Blue] = std::vector<bpp8_t>(width * height, 36u);

		ImageLayer<bpp8_t>::Params layerParams = {};
		layerParams.layerName = "Layer";
		layerParams.width = width;
		layerParams.height = height;

		auto layer = std::make_shared<ImageLayer<bpp8_t>>(
			std::move(channelMap),
			layerParams
		);
		document.addLayer(layer);

		File::FileParams params = File::FileParams();
		params.doRead = false;
		params.forceOverwrite = true;
		auto outputFile = File(psd_path, params);
		auto psdDocumentPtr = LayeredToPhotoshopFile(std::move(document));
		ProgressCallback callback{};
		psdDocumentPtr->write(outputFile, callback);
	}
	{
		auto inputFile = File(psd_path);
		auto psDocumentPtr = std::make_unique<PhotoshopFile>();
		ProgressCallback callback{};
		psDocumentPtr->read(inputFile, callback);
		LayeredFile<bpp8_t> layeredFile = { std::move(psDocumentPtr) };
		dpi = layeredFile.m_DotsPerInch;
	}

	CHECK(dpi == 300.0f);
}


TEST_CASE("Write DPI fractional")
{
	// We first write, then read the DPI after which we compare. We must store an image layer as well as Photoshop requires at least one
	// imagelayer.

	using namespace NAMESPACE_PSAPI;

	std::filesystem::path psd_path = std::filesystem::current_path();
	psd_path += "/documents/DPI/Write_700dpi.psd";

	float dpi = 0.0f;

	// Write a file with a 300 dpi
	{
		const uint32_t width = 64u;
		const uint32_t height = 64u;
		LayeredFile<bpp8_t> document = { Enum::ColorMode::RGB, width, height };
		document.m_DotsPerInch = 700.25f;

		std::unordered_map <Enum::ChannelID, std::vector<bpp8_t>> channelMap;
		channelMap[Enum::ChannelID::Red] = std::vector<bpp8_t>(width * height, 36u);
		channelMap[Enum::ChannelID::Green] = std::vector<bpp8_t>(width * height, 36u);
		channelMap[Enum::ChannelID::Blue] = std::vector<bpp8_t>(width * height, 36u);

		ImageLayer<bpp8_t>::Params layerParams = {};
		layerParams.layerName = "Layer";
		layerParams.width = width;
		layerParams.height = height;

		auto layer = std::make_shared<ImageLayer<bpp8_t>>(
			std::move(channelMap),
			layerParams
		);
		document.addLayer(layer);

		File::FileParams params = File::FileParams();
		params.doRead = false;
		params.forceOverwrite = true;
		auto outputFile = File(psd_path, params);
		auto psdDocumentPtr = LayeredToPhotoshopFile(std::move(document));
		ProgressCallback callback{};
		psdDocumentPtr->write(outputFile, callback);
	}
	{
		auto inputFile = File(psd_path);
		auto psDocumentPtr = std::make_unique<PhotoshopFile>();
		ProgressCallback callback{};
		psDocumentPtr->read(inputFile, callback);
		LayeredFile<bpp8_t> layeredFile = { std::move(psDocumentPtr) };
		dpi = layeredFile.m_DotsPerInch;
	}

	CHECK(dpi == 700.25f);
}