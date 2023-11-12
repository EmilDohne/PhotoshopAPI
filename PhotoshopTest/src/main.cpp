#include "Macros.h"
#include "PhotoshopFile/PhotoshopFile.h"

#include "Util/Struct/Signature.h"

#include <filesystem>
#include <vector>

std::vector<std::filesystem::path> relPaths =
{
	"\\documents\\CMYK_8bit.psd",
	"\\documents\\CMYK_8bit.psb",
	"\\documents\\CMYK_16bit.psd",
	"\\documents\\CMYK_16bit.psb",

	"\\documents\\Grayscale_8bit.psd",
	"\\documents\\Grayscale_8bit.psb",
	"\\documents\\Grayscale_16bit.psd",
	"\\documents\\Grayscale_16bit.psb",
	"\\documents\\Grayscale_32bit.psd",
	"\\documents\\Grayscale_32bit.psb",

	"\\documents\\Groups_8bit.psd",
	"\\documents\\Groups_8bit.psb",
	"\\documents\\Groups_16bit.psd",
	"\\documents\\Groups_16bit.psb",
	"\\documents\\Groups_32bit.psd",
	"\\documents\\Groups_32bit.psb",

	"\\documents\\Indexed_8bit.psd",
	"\\documents\\Indexed_8bit.psb",

	"\\documents\\Masks_8bit.psd",
	"\\documents\\Masks_8bit.psb",

	"\\documents\\SingleLayer_8bit.psd",
	"\\documents\\SingleLayer_8bit.psb",
	"\\documents\\SingleLayer_8bit_MaximizeCompatibilityOff.psd",
	"\\documents\\SingleLayer_8bit_MaximizeCompatibilityOff.psb",
	"\\documents\\SingleLayer_16bit.psd",
	"\\documents\\SingleLayer_16bit.psb",
	"\\documents\\SingleLayer_16bit_MaximizeCompatibilityOff.psd",
	"\\documents\\SingleLayer_16bit_MaximizeCompatibilityOff.psb",
	"\\documents\\SingleLayer_32bit.psd",
	"\\documents\\SingleLayer_32bit.psb",
	"\\documents\\SingleLayer_32bit_MaximizeCompatibilityOff.psd",
	"\\documents\\SingleLayer_32bit_MaximizeCompatibilityOff.psb",
};


int main()
{
	std::filesystem::path currentDirectory = std::filesystem::current_path();

	for (const auto& path : relPaths)
	{
		std::filesystem::path combined_path = currentDirectory;
		combined_path += path;
		NAMESPACE_PSAPI::File file(combined_path);
		NAMESPACE_PSAPI::PhotoshopFile document;
		bool didParse = document.read(file);

		if (didParse)
		{
			PSAPI_LOG("PhotoshopTest", "Successfully finished parsing of file %s", path.string().c_str());
		}
		else
		{
			PSAPI_LOG("PhotoshopTest", "Failed parsing of file %s", path.string().c_str());
		}
	}
}