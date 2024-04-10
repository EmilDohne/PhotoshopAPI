#include "doctest.h"

#include "Utility.h"
#include "Macros.h"
#include "Compression/Decompress_RLE.h"
#include "Compression/Compress_RLE.h"
#include "PhotoshopFile/PhotoshopFile.h"

#include <vector>
#include <filesystem>


// This tests the sample provided in the wikipedia page for the packbits algorithm and this is the exact implementation used
// by photoshop as well
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Test Wikipedia Packbits Example")
{
	// Equates to 'FE AA 02 80 00 2A FD AA 03 80 00 2A 22 F7 AA' in hexadecimal
	std::vector<uint8_t> data = { 254u, 170u, 2u, 128u, 0u, 42u, 253u, 170u, 3u, 128u, 0u, 42u, 34u, 247u, 170u };
	std::vector<uint8_t> expected = { 170u, 170u, 170u, 128u, 0u, 42u, 170u, 170u, 170u, 170u, 128u, 0u, 42u, 34u, 170u, 170u, 170u, 170u, 170u, 170u, 170u, 170u, 170u, 170u };


	std::vector<uint8_t> result = NAMESPACE_PSAPI::RLE_Impl::DecompressPackBits<uint8_t>(data, expected.size(), 1u);
	CHECK(result == expected);

}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Decompress file with RLE compression")
{
	SUBCASE("PSD")
	{
		std::filesystem::path combined_path = std::filesystem::current_path() / "documents/Compression/Compression_RLE_8bit.psd";
		checkDecompressionFile<uint8_t>(combined_path, 0, 128, 255, 0);
	}
	SUBCASE("PSB")
	{
		std::filesystem::path combined_path = std::filesystem::current_path() / "documents/Compression/Compression_RLE_8bit.psb";
		checkDecompressionFile<uint8_t>(combined_path, 0, 128, 255, 0);
	}
}