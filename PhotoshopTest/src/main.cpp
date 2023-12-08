// Include doctest and configure our own main function
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include "Macros.h"
#include "Enum.h"
#include "PhotoshopFile/PhotoshopFile.h"
#include "LayeredFile/LayeredFile.h"

#include "Profiling/Perf/Instrumentor.h"
#include "Profiling/Memory/CompressionTracker.h"

#include <filesystem>
#include <vector>
#include <memory>

// This is the data we run all the whole-file tests on. Keep in mind that this does not necessarily cover all of the documents found in /documents
// as some cover very specific individual sections such as /documents/Compression
std::vector<std::filesystem::path> relPaths =
{
	/*"\\documents\\CMYK\\CMYK_8bit.psd",
	"\\documents\\CMYK\\CMYK_8bit.psb",
	"\\documents\\CMYK\\CMYK_16bit.psd",
	"\\documents\\CMYK\\CMYK_16bit.psb",

	"\\documents\\Grayscale\\Grayscale_8bit.psd",
	"\\documents\\Grayscale\\Grayscale_8bit.psb",
	"\\documents\\Grayscale\\Grayscale_16bit.psd",
	"\\documents\\Grayscale\\Grayscale_16bit.psb",
	"\\documents\\Grayscale\\Grayscale_32bit.psd",
	"\\documents\\Grayscale\\Grayscale_32bit.psb",

	"\\documents\\Groups\\Groups_8bit.psd",
	"\\documents\\Groups\\Groups_8bit.psb",
	"\\documents\\Groups\\Groups_16bit.psd",
	"\\documents\\Groups\\Groups_16bit.psb",
	"\\documents\\Groups\\Groups_32bit.psd",
	"\\documents\\Groups\\Groups_32bit.psb",

	"\\documents\\Indexed\\Indexed_8bit.psd",
	"\\documents\\Indexed\\Indexed_8bit.psb",

	"\\documents\\Masks\\Masks_8bit.psd",
	"\\documents\\Masks\\Masks_8bit.psb",

	"\\documents\\SingleLayer\\SingleLayer_8bit.psd",
	"\\documents\\SingleLayer\\SingleLayer_8bit.psb",
	"\\documents\\SingleLayer\\SingleLayer_8bit_MaximizeCompatibilityOff.psd",
	"\\documents\\SingleLayer\\SingleLayer_8bit_MaximizeCompatibilityOff.psb",
	"\\documents\\SingleLayer\\SingleLayer_16bit.psd",
	"\\documents\\SingleLayer\\SingleLayer_16bit.psb",
	"\\documents\\SingleLayer\\SingleLayer_16bit_MaximizeCompatibilityOff.psd",
	"\\documents\\SingleLayer\\SingleLayer_16bit_MaximizeCompatibilityOff.psb",
	"\\documents\\SingleLayer\\SingleLayer_32bit.psd",
	"\\documents\\SingleLayer\\SingleLayer_32bit.psb",
	"\\documents\\SingleLayer\\SingleLayer_32bit_MaximizeCompatibilityOff.psd",
	"\\documents\\SingleLayer\\SingleLayer_32bit_MaximizeCompatibilityOff.psb",*/
	"\\documents\\tmp.psb"
};


void profile()
{
	// Initialize our Instrumentor instance here to write out our profiling info
	NAMESPACE_PSAPI::Instrumentor::Get().BeginSession("PSAPI_Profile", "parallel_cblosc.json");
	NAMESPACE_PSAPI::CompressionTracker::Get().BeginSession("PSAPI_Profile");

	std::filesystem::path currentDirectory = std::filesystem::current_path();

	for (const auto& path : relPaths)
	{
		std::filesystem::path combined_path = currentDirectory;
		combined_path += path;

		PSAPI_LOG("Main", "Started Parsing of file %s", combined_path.string().c_str())

		NAMESPACE_PSAPI::File file(combined_path);
		std::unique_ptr<NAMESPACE_PSAPI::PhotoshopFile> document = std::make_unique<NAMESPACE_PSAPI::PhotoshopFile>();
		bool didParse = document->read(file);

		if (didParse)
		{
			PSAPI_LOG("PhotoshopTest", "Successfully finished parsing of file %s", path.string().c_str());
		}
		else
		{
			PSAPI_LOG("PhotoshopTest", "Failed parsing of file %s", path.string().c_str());
		}
		// Generate our layeredFiles
		if (document->m_Header.m_Depth == NAMESPACE_PSAPI::Enum::BitDepth::BD_8)
		{
			NAMESPACE_PSAPI::LayeredFile<uint8_t> layeredFile(std::move(document));
		}
		else if (document->m_Header.m_Depth == NAMESPACE_PSAPI::Enum::BitDepth::BD_16)
		{
			NAMESPACE_PSAPI::LayeredFile<uint16_t> layeredFile(std::move(document));
		}
		else if (document->m_Header.m_Depth == NAMESPACE_PSAPI::Enum::BitDepth::BD_16)
		{
			NAMESPACE_PSAPI::LayeredFile<float32_t> layeredFile(std::move(document));
		}
	}

	NAMESPACE_PSAPI::CompressionTracker::Get().EndSession();
	NAMESPACE_PSAPI::Instrumentor::Get().EndSession();
}

int main()
{
	// Profile and test our application all in one step
	profile();

	// Set up and run doctest tests
	{
		doctest::Context context;

		// set defaults
		context.setOption("abort-after", 5);	// stop test execution after 5 failed assertions

		int res = context.run();				// run

		if (context.shouldExit())				// important - query flags (and --exit) rely on the user doing this
			return res;							// propagate the result of the tests
	}
}