#include "doctest.h"

#include "Utility.h"
#include "Compression/RLE.h"


#include <vector>


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Test PackBits roundtripping")
{
	std::vector<uint8_t> data(256);
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < data.size() / 4; ++j)
		{
			data[(data.size() / 4) * i + j] = i;
		}
	}
	uint32_t scanlineSize = 0u;	// Defines the size of the compressed scanline

	std::vector<uint8_t> compressed = NAMESPACE_PSAPI::CompressPackBits(std::span<uint8_t>(data.data(), data.size()), scanlineSize);
	std::vector<uint8_t> uncompressed = NAMESPACE_PSAPI::DecompressPackBits<uint8_t>(compressed, data.size(), 1u);
	CHECK(uncompressed == data);
	CHECK(uncompressed.size() == data.size());
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Test Wikipedia Example")
{
	// Equates to 'FE AA 02 80 00 2A FD AA 03 80 00 2A 22 F7 AA' in hexadecimal
	std::vector<uint8_t> data = { 170u, 170u, 170u, 128u, 0u, 42u, 170u, 170u, 170u, 170u, 128u, 0u, 42u, 34u, 170u, 170u, 170u, 170u, 170u, 170u, 170u, 170u, 170u, 170u };
	// We insert an extra 128u at the end of the sequence here since we actually 
	std::vector<uint8_t> expected = { 254u, 170u, 2u, 128u, 0u, 42u, 253u, 170u, 3u, 128u, 0u, 42u, 34u, 247u, 170u, 128u };


	uint32_t scanlineSize = 0u;	// Defines the size of the compressed scanline
	std::vector<uint8_t> compressed = NAMESPACE_PSAPI::CompressPackBits(std::span<uint8_t>(data.data(), data.size()), scanlineSize);

	CHECK(compressed == expected);
	CHECK(compressed.size() == expected.size());
}


// Test that we can read, decompress and then recompress image data and get the exact same result. These tests rely on the
// TestDecompressionTests to pass successfully
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Photoshop File Roundtripping")
{
	SUBCASE("PSD")
	{
		std::filesystem::path combined_path = std::filesystem::current_path() / "documents/Compression/Compression_RLE_8bit.psd";
		checkCompressionFile<uint8_t>(combined_path);
	}
	SUBCASE("PSB")
	{
		std::filesystem::path combined_path = std::filesystem::current_path() / "documents/Compression/Compression_RLE_8bit.psb";
		checkCompressionFile<uint8_t>(combined_path);
	}
}