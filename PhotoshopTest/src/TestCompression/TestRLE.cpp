#include "doctest.h"

#include "Utility.h"
#include "Macros.h"
#include "Compression/RLE.h"
#include "PhotoshopFile/PhotoshopFile.h"

#include <vector>
#include <filesystem>


// This tests the sample provided in the wikipedia page for the packbits algorithm and this is the exact implementation used
// by photoshop as well
TEST_CASE("Test Wikipedia Packbits Example")
{
	// Equates to 'FE AA 02 80 00 2A FD AA 03 80 00 2A 22 F7 AA' in hexadecimal
	std::vector<uint8_t> data = { 254u, 170u, 2u, 128u, 0u, 42u, 253u, 170u, 3u, 128u, 0u, 42u, 34u, 247u, 170u };
	std::vector<uint8_t> expected = { 170u, 170u, 170u, 128u, 0u, 42u, 170u, 170u, 170u, 170u, 128u, 0u, 42u, 34u, 170u, 170u, 170u, 170u, 170u, 170u, 170u, 170u, 170u, 170u };

	SUBCASE("Defining no width and height")
	{
		// Note that we do not need to actually provide a width and height here as that only allows for premature reserving of the data
		// but is not strictly necessary
		std::vector<uint8_t> resultNoSize = NAMESPACE_PSAPI::DecompressPackBits<uint8_t>(data, 0u, 0u);
		CHECK(resultNoSize == expected);
	}

	SUBCASE("Defining too large width and height")
	{
		std::vector<uint8_t> resultTooLarge = NAMESPACE_PSAPI::DecompressPackBits<uint8_t>(data, 128u, 128u);
		CHECK(resultTooLarge == expected);
	}
}


TEST_CASE("Decompress file with RLE compression")
{
	SUBCASE("PSD")
	{
		std::filesystem::path combined_path = std::filesystem::current_path();
		combined_path += "\\documents\\Compression\\Compression_RLE_8bit.psd";
		checkCompressionFile<uint8_t>(combined_path, 0, 128, 255, 0);
	}
	SUBCASE("PSB")
	{
		std::filesystem::path combined_path = std::filesystem::current_path();
		combined_path += "\\documents\\Compression\\Compression_RLE_8bit.psb";
		checkCompressionFile<uint8_t>(combined_path, 0, 128, 255, 0);
	}
}