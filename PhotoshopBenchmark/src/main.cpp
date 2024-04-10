
#include "PhotoshopAPI.h"
#include "Profiler.h"

#include <filesystem>
#include <string>

const static std::filesystem::path outStats = "benchmarkStatisticsPSAPI.txt";


// Read and write the file while changing the compression type to zip to get smaller files
template <typename T>
void readWriteFile(const int repeats, const std::filesystem::path& readPath, const std::filesystem::path& writePath, const std::string& benchName)
{
	using namespace NAMESPACE_PSAPI;

	for (int i = 0; i < repeats; ++i)
	{
		// Run a dry-run first which we do not profile to warm up
		if (i == 0)
		{
			LayeredFile<T> layeredFile;
			auto inputFile = File(readPath);
			auto psDocumentPtr = std::make_unique<PhotoshopFile>();
			psDocumentPtr->read(inputFile);
			layeredFile = { std::move(psDocumentPtr) };
			continue;
		}
		Instrumentor::Get().BeginSession((readPath.string() + std::to_string(i)).c_str(), (readPath.string() + std::to_string(i) + ".json").c_str());
		LayeredFile<T> layeredFile;
		{
			Profiler readProfiler{ outStats , "read" + benchName };
			// Load the input file
			auto inputFile = File(readPath);
			auto psDocumentPtr = std::make_unique<PhotoshopFile>();
			psDocumentPtr->read(inputFile);
			layeredFile = { std::move(psDocumentPtr) };
		}
		{
			Profiler writeProfiler{ outStats , "write" + benchName };

			// Write to disk
			File::FileParams params = File::FileParams();
			params.doRead = false;
			params.forceOverwrite = true;
			auto outputFile = File(writePath, params);
			auto psdOutDocumentPtr = LayeredToPhotoshopFile(std::move(layeredFile));
			psdOutDocumentPtr->write(outputFile);
		}
		Instrumentor::Get().EndSession();
	}
}


// Read and write the file while changing the compression type to zip to get smaller files
template <typename T>
void readWriteFileChangeCompression(const int repeats, const std::filesystem::path& readPath, const std::filesystem::path& writePath, const std::string& benchName)
{
	using namespace NAMESPACE_PSAPI;

	for (int i = 0; i < repeats + 1; ++i)
	{
		// Run a dry-run first which we do not profile to warm up
		if (i == 0)
		{
			LayeredFile<T> layeredFile;
			auto inputFile = File(readPath);
			auto psDocumentPtr = std::make_unique<PhotoshopFile>();
			psDocumentPtr->read(inputFile);
			layeredFile = { std::move(psDocumentPtr) };
			continue;
		}
		LayeredFile<T> layeredFile;
		{
			Profiler readProfiler{ outStats , "read" + benchName };
			// Load the input file
			auto inputFile = File(readPath);
			auto psDocumentPtr = std::make_unique<PhotoshopFile>();
			psDocumentPtr->read(inputFile);
			layeredFile = { std::move(psDocumentPtr) };
		}
		{
			Profiler writeProfiler{ outStats , "write" + benchName };
			layeredFile.setCompression(Enum::Compression::Zip);

			// Write to disk
			File::FileParams params = File::FileParams();
			params.doRead = false;
			params.forceOverwrite = true;
			auto outputFile = File(writePath, params);
			auto psdOutDocumentPtr = LayeredToPhotoshopFile(std::move(layeredFile));
			psdOutDocumentPtr->write(outputFile);
		}
	}
}



int main()
{
	using namespace NAMESPACE_PSAPI;
	static const int repeats = 3u;

	// These files are just here to test the size of single layer photoshop files
	//readWriteFile<bpp16_t>(1u, "documents/read/single_layer_16bit.psb", "documents/write/single_layer_16bit.psb", "single_layer_16bit");
	//readWriteFile<bpp32_t>(1u, "documents/read/single_layer_32bit.psb", "documents/write/single_layer_32bit.psb", "single_layer_32bit");

	readWriteFile<bpp8_t>(repeats, "documents/read/large_file_8bit.psb", "documents/write/large_file_8bit.psb", "Automotive Data (8-bit) ~1.27GB");
	// Benchmark how changing the compression reduces the file size
	readWriteFileChangeCompression<bpp8_t>(repeats, "documents/read/large_file_8bit.psb", "documents/write/large_fileZip_8bit.psb", "Automotive Data Zip (8-bit) ~1.27GB");
	readWriteFile<bpp16_t>(repeats, "documents/read/large_file_16bit.psb", "documents/write/large_file_16bit.psb", "Automotive Data (16-bit) ~1.97GB");
	readWriteFile<bpp32_t>(repeats, "documents/read/large_file_32bit.psb", "documents/write/large_file_32bit.psb", "Automotive Data (32-bit) ~3.65GB");
	readWriteFile<bpp8_t>(repeats, "documents/read/HyundaiGenesis_GlaciusCreations_8bit.psd", "documents/write/HyundaiGenesis_GlaciusCreations_8bit.psd", "Glacius Hyundai Sample (8-bit) ~.75GB");
	readWriteFile<bpp8_t>(repeats, "documents/read/deep_nesting_8bit.psb", "documents/write/deep_nesting_8bit.psb", "Deep Nested Layers (8-bit) ~.5GB");
	// Benchmark how changing the compression reduces the file size
	readWriteFileChangeCompression<bpp8_t>(repeats, "documents/read/HyundaiGenesis_GlaciusCreations_8bit.psd", "documents/write/HyundaiGenesis_GlaciusCreationsZip_8bit.psd", "Glacius Hyundai Sample Zip (8-bit) ~.75GB");
}