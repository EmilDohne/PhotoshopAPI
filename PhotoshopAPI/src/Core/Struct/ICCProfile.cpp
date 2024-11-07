#include "ICCProfile.h"

#include "Core/FileIO/Read.h"

#include <filesystem>
#include <string>

PSAPI_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
ICCProfile::ICCProfile(const std::filesystem::path& pathToICCFile)
{
	if (pathToICCFile.extension() != ".icc")
	{
		PSAPI_LOG_ERROR("ICCProfile", "Must pass a valid .icc file into the ctor. Got a %s", pathToICCFile.extension().string().c_str());
	}
		// Open a File object and read the raw bytes of the ICC file
	File iccFile = { pathToICCFile };
	m_Data = ReadBinaryArray<uint8_t>(iccFile, iccFile.getSize());
}

PSAPI_NAMESPACE_END