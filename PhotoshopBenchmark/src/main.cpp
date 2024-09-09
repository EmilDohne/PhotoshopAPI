
#include "PhotoshopAPI.h"
#include "Profiler.h"

#include <filesystem>
#include <string>
#include <iostream>

const static std::filesystem::path outStats = "benchmarkStatisticsPSAPI.txt";


// Read and write the file while changing the compression type to zip to get smaller files
template <typename T>
void readWriteFile(const int repeats, const std::filesystem::path& readPath, const std::filesystem::path& writePath, const std::string& benchName)
{
	using namespace NAMESPACE_PSAPI;

	for (int i = 0; i < repeats + 1; ++i)
	{
		// Run a dry-run first which we do not profile to warm up
		if (i == 0)
		{
			auto res = LayeredFile<T>::read(readPath);
			continue;
		}
		Instrumentor::Get().BeginSession((readPath.string() + std::to_string(i)).c_str(), (readPath.string() + std::to_string(i) + ".json").c_str());
		LayeredFile<T> layeredFile;
		{
			Profiler readProfiler{ outStats , "read" + benchName };
			layeredFile = LayeredFile<T>::read(readPath);
		}
		{
			Profiler writeProfiler{ outStats , "write" + benchName };
			LayeredFile<T>::write(std::move(layeredFile), writePath);
		}
		Instrumentor::Get().EndSession();
	}
}


// Read the file and benchmark layer extraction speeds
template <typename T>
void readFileExtractData(const int repeats, const std::filesystem::path& readPath, const std::filesystem::path& writePath, const std::string& benchName)
{
	using namespace NAMESPACE_PSAPI;

	for (int i = 0; i < repeats + 1; ++i)
	{
		// Run a dry-run first which we do not profile to warm up
		if (i == 0)
		{
			auto res = LayeredFile<T>::read(readPath);
			continue;
		}
		Instrumentor::Get().BeginSession(readPath.string() + "Extract" + std::to_string(i), readPath.string() + "Extract" + std::to_string(i) + ".json");
		LayeredFile<T> layeredFile;
		{
			Profiler readProfiler{ outStats , "read" + benchName };
			layeredFile = LayeredFile<T>::read(readPath);
		}
		{
			// Extract all the flat layers and get the image data out of any image layers. While in this case it would be trivial to parallelize this to use up
			// all threads we want to simulate an environment where this extraction doesnt happen in one go
			Profiler extractProfiler{ outStats, "extract" + benchName };
			for (const auto& layer : layeredFile.generateFlatLayers(std::nullopt, LayerOrder::forward))
			{
				if (auto imageLayer = dynamic_pointer_cast<ImageLayer<T>>(layer))
				{
					extractProfiler.startTimePoint();
					auto data = imageLayer->getImageData();
					extractProfiler.endTimePoint();
				}
			}
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
			auto res = LayeredFile<T>::read(readPath);
			continue;
		}
		LayeredFile<T> layeredFile;
		{
			Profiler readProfiler{ outStats , "read" + benchName };
			layeredFile = LayeredFile<T>::read(readPath);
		}
		{
			Profiler writeProfiler{ outStats , "write" + benchName };
			layeredFile.setCompression(Enum::Compression::Zip);
			LayeredFile<T>::write(std::move(layeredFile), writePath);
		}
	}
}



int main(int argc, char* argv[])
{
	using namespace NAMESPACE_PSAPI;
	static const int repeats = 3u;

	// Tokenize the arguments
	std::vector<std::string> tokens;
	for (int i = 1; i < argc; ++i)
	{
		tokens.push_back(std::string(argv[i]));
	}

	// Run only the extraction part of the test
	if (std::find(tokens.begin(), tokens.end(), "--extract-only") != tokens.end())
	{
		// Benchmark how fast extraction of layers is
		readFileExtractData<bpp8_t>(repeats, "documents/read/large_file_8bit.psb", "documents/write/large_file_8bit.psb", "Automotive Data (8-bit) ~1.27GB");
		readFileExtractData<bpp16_t>(repeats, "documents/read/large_file_16bit.psb", "documents/write/large_file_16bit.psb", "Automotive Data (16-bit) ~1.97GB");
		readFileExtractData<bpp32_t>(repeats, "documents/read/large_file_32bit.psb", "documents/write/large_file_32bit.psb", "Automotive Data (32-bit) ~3.65GB");
		readFileExtractData<bpp8_t>(repeats, "documents/read/HyundaiGenesis_GlaciousCreations_8bit.psd", "documents/write/HyundaiGenesis_GlaciousCreations_8bit.psd", "Glacious Hyundai Sample (8-bit) ~.75GB");
		readFileExtractData<bpp8_t>(repeats, "documents/read/deep_nesting_8bit.psb", "documents/write/deep_nesting_8bit.psb", "Deep Nested Layers (8-bit) ~.5GB");
		return 0;
	}


	// These files are just here to test the size of single layer photoshop files
	//readWriteFile<bpp16_t>(1u, "documents/read/single_layer_16bit.psb", "documents/write/single_layer_16bit.psb", "single_layer_16bit");
	//readWriteFile<bpp32_t>(1u, "documents/read/single_layer_32bit.psb", "documents/write/single_layer_32bit.psb", "single_layer_32bit");

	readWriteFile<bpp8_t>(repeats, "documents/read/large_file_8bit.psb", "documents/write/large_file_8bit.psb", "Automotive Data (8-bit) ~1.27GB");
	// Benchmark how changing the compression reduces the file size
	readWriteFileChangeCompression<bpp8_t>(repeats, "documents/read/large_file_8bit.psb", "documents/write/large_fileZip_8bit.psb", "Automotive Data Zip (8-bit) ~1.27GB");
	readWriteFile<bpp16_t>(repeats, "documents/read/large_file_16bit.psb", "documents/write/large_file_16bit.psb", "Automotive Data (16-bit) ~1.97GB");
	readWriteFile<bpp32_t>(repeats, "documents/read/large_file_32bit.psb", "documents/write/large_file_32bit.psb", "Automotive Data (32-bit) ~3.65GB");
	readWriteFile<bpp8_t>(repeats, "documents/read/HyundaiGenesis_GlaciousCreations_8bit.psd", "documents/write/HyundaiGenesis_GlaciousCreations_8bit.psd", "Glacious Hyundai Sample (8-bit) ~.75GB");
	readWriteFile<bpp8_t>(repeats, "documents/read/deep_nesting_8bit.psb", "documents/write/deep_nesting_8bit.psb", "Deep Nested Layers (8-bit) ~.5GB");
	// Benchmark how changing the compression reduces the file size
	readWriteFileChangeCompression<bpp8_t>(repeats, "documents/read/HyundaiGenesis_GlaciousCreations_8bit.psd", "documents/write/HyundaiGenesis_GlaciousCreationsZip_8bit.psd", "Glacious Hyundai Sample Zip (8-bit) ~.75GB");

	
}