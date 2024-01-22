#include "PhotoshopAPI.h"


template <typename T>
void createDocument(const uint32_t size, double one_val)
{
	// Initialize some constants that we will need throughout the program
	const static PhotoshopAPI::Enum::ColorMode colorMode = PhotoshopAPI::Enum::ColorMode::RGB;

	PhotoshopAPI::LayeredFile<T> document = { colorMode, size, size };

	for (int i = 0; i < 50; ++i)
	{
		// Create our individual channels to add to our image layer. Keep in mind that all these 3 channels need to 
		// be specified for RGB mode
		std::unordered_map <PhotoshopAPI::Enum::ChannelID, std::vector<T>> channelMap;
		channelMap[PhotoshopAPI::Enum::ChannelID::Red] = std::vector<T>(static_cast<uint64_t>(size) * size, one_val);
		channelMap[PhotoshopAPI::Enum::ChannelID::Green] = std::vector<T>(static_cast<uint64_t>(size) * size, 0);
		channelMap[PhotoshopAPI::Enum::ChannelID::Blue] = std::vector<T>(static_cast<uint64_t>(size) * size, 0);

		PhotoshopAPI::ImageLayer<T>::template Params layerParams = {};
		layerParams.layerName = "Layer Red";
		layerParams.width = size;
		layerParams.height = size;
		layerParams.colorMode = colorMode;
		layerParams.compression = PhotoshopAPI::Enum::Compression::ZipPrediction;

		auto layer = std::make_shared<PhotoshopAPI::ImageLayer<T>>(std::move(channelMap), std::nullopt, layerParams);
		document.addLayer(layer);
	}

	// Convert to PhotoshopDocument and write to disk.
	if (sizeof(T) == 1)
	{
		auto outputFile = PhotoshopAPI::File("./WriteSimpleFile_" + std::to_string(size) + "_8.psd", false, true);
		auto psdDocumentPtr = PhotoshopAPI::LayeredToPhotoshopFile(std::move(document));
		psdDocumentPtr->write(outputFile);
	}
	else if (sizeof(T) == 2)
	{
		auto outputFile = PhotoshopAPI::File("./WriteSimpleFile_" + std::to_string(size) + "_16.psd", false, true);
		auto psdDocumentPtr = PhotoshopAPI::LayeredToPhotoshopFile(std::move(document));
		psdDocumentPtr->write(outputFile);
	}
	else if (sizeof(T) == 4)
	{
		auto outputFile = PhotoshopAPI::File("./WriteSimpleFile_" + std::to_string(size) + "_32.psd", false, true);
		auto psdDocumentPtr = PhotoshopAPI::LayeredToPhotoshopFile(std::move(document));
		psdDocumentPtr->write(outputFile);
	}
}


template <typename T>
void readDocument(const uint32_t size)
{

	if (sizeof(T) == 1)
	{
		auto input = PhotoshopAPI::File("./WriteSimpleFile_" + std::to_string(size) + "_8.psd");
		std::unique_ptr<NAMESPACE_PSAPI::PhotoshopFile> document = std::make_unique<NAMESPACE_PSAPI::PhotoshopFile>();
		document->read(input);
	}
	else if (sizeof(T) == 2)
	{
		auto input = PhotoshopAPI::File("./WriteSimpleFile_" + std::to_string(size) + "_16.psd");
		std::unique_ptr<NAMESPACE_PSAPI::PhotoshopFile> document = std::make_unique<NAMESPACE_PSAPI::PhotoshopFile>();
		document->read(input);
	}
	else if (sizeof(T) == 4)
	{
		auto input = PhotoshopAPI::File("./WriteSimpleFile_" + std::to_string(size) + "_32.psd");
		std::unique_ptr<NAMESPACE_PSAPI::PhotoshopFile> document = std::make_unique<NAMESPACE_PSAPI::PhotoshopFile>();
		document->read(input);
	}
}

int main()
{
	//// Initialize some constants that we will need throughout the program
	//const static PhotoshopAPI::Enum::ColorMode colorMode = PhotoshopAPI::Enum::ColorMode::RGB;
	//const static uint32_t width = 64u;
	//const static uint32_t height = 64u;

	//PhotoshopAPI::LayeredFile<PhotoshopAPI::bpp8_t> document = { colorMode, width, height };

	//// Create our individual channels to add to our image layer. Keep in mind that all these 3 channels need to 
	//// be specified for RGB mode
	//std::unordered_map <PhotoshopAPI::Enum::ChannelID, std::vector<uint8_t>> channelMap;
	//channelMap[PhotoshopAPI::Enum::ChannelID::Red] = std::vector<uint8_t>(width * height, 255u);
	//channelMap[PhotoshopAPI::Enum::ChannelID::Green] = std::vector<uint8_t>(width * height, 0u);
	//channelMap[PhotoshopAPI::Enum::ChannelID::Blue] = std::vector<uint8_t>(width * height, 0u);

	//PhotoshopAPI::ImageLayer<PhotoshopAPI::bpp8_t>::Params layerParams = {};
	//layerParams.layerName = "Layer Red";
	//layerParams.width = width;
	//layerParams.height = height;

	//auto layer = std::make_shared<PhotoshopAPI::ImageLayer<PhotoshopAPI::bpp8_t>>(std::move(channelMap), std::nullopt, layerParams);
	//document.addLayer(layer);

	//// It is perfectly legal to modify a layers properties even after it was added to the document as attributes
	//// are only finalized on export
	//layer->m_Opacity = 128u;

	//// Convert to PhotoshopDocument and write to disk. Note that from this point onwards 
	//// our LayeredFile instance is no longer usable
	//auto outputFile = PhotoshopAPI::File("./WriteSimpleFile.psd", false, true);
	//auto psdDocumentPtr = PhotoshopAPI::LayeredToPhotoshopFile(std::move(document));
	//psdDocumentPtr->write(outputFile);

	// Initialize our Instrumentor instance here to write out our profiling info
	PhotoshopAPI::Instrumentor::Get().BeginSession("PSAPI_Profile", "sizes.json");

	std::vector<uint32_t> sizes = {8192};
	for (const auto size : sizes)
	{
		createDocument<PhotoshopAPI::bpp8_t>(size, 255u);
		createDocument<PhotoshopAPI::bpp16_t>(size, 65535u);
		createDocument<PhotoshopAPI::bpp32_t>(size, 1.0f);
	}

	PhotoshopAPI::Instrumentor::Get().EndSession();
}