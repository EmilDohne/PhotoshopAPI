
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

#pragma push_macro("PSAPI_PROFILING")
#define PSAPI_PROFILING 0

	for (int i = 0; i < repeats; ++i)
	{
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
			File::FileParams params = { .doRead = false, .forceOverwrite = true };
			auto outputFile = File(writePath, params);
			auto psdOutDocumentPtr = LayeredToPhotoshopFile(std::move(layeredFile));
			psdOutDocumentPtr->write(outputFile);
		}
	}
#pragma pop_macro("PSAPI_PROFILING")
}


// Read and write the file while changing the compression type to zip to get smaller files
template <typename T>
void readWriteFileChangeCompression(const int repeats, const std::filesystem::path& readPath, const std::filesystem::path& writePath, const std::string& benchName)
{
	using namespace NAMESPACE_PSAPI;

#pragma push_macro("PSAPI_PROFILING")
#define PSAPI_PROFILING 0

	for (int i = 0; i < repeats; ++i)
	{
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
			File::FileParams params = { .doRead = false, .forceOverwrite = true };
			auto outputFile = File(writePath, params);
			auto psdOutDocumentPtr = LayeredToPhotoshopFile(std::move(layeredFile));
			psdOutDocumentPtr->write(outputFile);
		}
	}
#pragma pop_macro("PSAPI_PROFILING")
}



int main()
{
	using namespace NAMESPACE_PSAPI;
	static const int repeats = 3u;

	readWriteFile<bpp8_t>(repeats, "documents/read/large_file_8bit.psb", "documents/write/large_file_8bit.psb", "Automotive Data (8-bit) ~1.27GB");
	//// Benchmark how changing the compression reduces the file size
	readWriteFileChangeCompression<bpp8_t>(repeats, "documents/read/large_file_8bit.psb", "documents/write/large_fileZip_8bit.psb", "Automotive Data Zip (8-bit) ~1.27GB");
	readWriteFile<bpp16_t>(repeats, "documents/read/large_file_16bit.psb", "documents/write/large_file_16bit.psb", "Automotive Data (16-bit) ~1.97GB");
	readWriteFile<bpp32_t>(repeats, "documents/read/large_file_32bit.psb", "documents/write/large_file_32bit.psb", "Automotive Data (32-bit) ~3.65GB");
	readWriteFile<bpp8_t>(repeats, "documents/read/HyundaiGenesis_GlaciousCreations_8bit.psd", "documents/write/HyundaiGenesis_GlaciousCreations_8bit.psd", "Glacious Hyundai Sample (8-bit) ~.75GB");
	readWriteFile<bpp8_t>(repeats, "documents/read/deep_nesting_8bit.psb", "documents/write/deep_nesting_8bit.psb", "Deep Nested Layers (8-bit) ~.5GB");
	// Benchmark how changing the compression reduces the file size
	readWriteFileChangeCompression<bpp8_t>(repeats, "documents/read/HyundaiGenesis_GlaciousCreations_8bit.psd", "documents/write/HyundaiGenesis_GlaciousCreationsZip_8bit.psd", "Glacious Hyundai Sample Zip (8-bit) ~.75GB");
}