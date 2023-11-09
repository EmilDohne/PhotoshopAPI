#include "Macros.h"
#include "PhotoshopFile/PhotoshopFile.h"

#include "Util/Struct/Signature.h"

#include <filesystem>
#include <vector>

std::vector<std::filesystem::path> relPaths =
{
	"\\documents\\Masks_8bit.psd",
	"\\documents\\Masks_8bit.psb"
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