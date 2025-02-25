#include "doctest.h"

#include "PhotoshopFile/PhotoshopFile.h"
#include "LayeredFile/LayeredFile.h"
#include "Macros.h"

#include <filesystem>

/*
It appears that Photoshop uses slightly different internal ICC profiles which match the ones on disk in terms of size but changes around some data. These seem
to be baked into some binary data therefore these ICC profiles we have are extracted straight from the PSBs
*/


TEST_CASE("Read AdobeRGB1998")
{
	using namespace NAMESPACE_PSAPI;

	std::filesystem::path psb_path = std::filesystem::current_path();
	psb_path += "/documents/ICCProfiles/AdobeRGB1998.psb";

	std::filesystem::path icc_path = std::filesystem::current_path();
	icc_path += "/documents/ICCProfiles/AdobeRGB1998.icc";

	LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read(psb_path);

	// Get the ICC Profile we read from the PSB
	std::vector<uint8_t> readICCProfile = layeredFile.icc_profile().data();

	// Get the ICC profile directly from disk
	File iccFile = { icc_path };
	std::vector<uint8_t> diskICCData = ReadBinaryArray<uint8_t>(iccFile, iccFile.getSize());

	CHECK(readICCProfile == diskICCData);
}


TEST_CASE("Read AppleRGB")
{
	using namespace NAMESPACE_PSAPI;

	std::filesystem::path psb_path = std::filesystem::current_path();
	psb_path += "/documents/ICCProfiles/AppleRGB.psb";

	std::filesystem::path icc_path = std::filesystem::current_path();
	icc_path += "/documents/ICCProfiles/AppleRGB.icc";

	LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read(psb_path);

	// Get the ICC Profile we read from the PSB
	std::vector<uint8_t> readICCProfile = layeredFile.icc_profile().data();

	// Get the ICC profile directly from disk
	File iccFile = { icc_path };
	std::vector<uint8_t> diskICCData = ReadBinaryArray<uint8_t>(iccFile, iccFile.getSize());

	CHECK(readICCProfile == diskICCData);
}


TEST_CASE("Read CIERGB")
{
	using namespace NAMESPACE_PSAPI;

	std::filesystem::path psb_path = std::filesystem::current_path();
	psb_path += "/documents/ICCProfiles/CIERGB.psb";

	std::filesystem::path icc_path = std::filesystem::current_path();
	icc_path += "/documents/ICCProfiles/CIERGB.icc";

	LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read(psb_path);

	// Get the ICC Profile we read from the PSB
	std::vector<uint8_t> readICCProfile = layeredFile.icc_profile().data();

	// Get the ICC profile directly from disk
	File iccFile = { icc_path };
	std::vector<uint8_t> diskICCData = ReadBinaryArray<uint8_t>(iccFile, iccFile.getSize());

	CHECK(readICCProfile == diskICCData);
}