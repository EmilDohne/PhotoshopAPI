#include "doctest.h"

#include "Utility.h"
#include "Core/Compression/Compress_RLE.h"
#include "Core/Compression/Decompress_RLE.h"


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

	std::vector<uint8_t> compressed = NAMESPACE_PSAPI::RLE_Impl::CompressPackBits(std::span<uint8_t>(data.data(), data.size()), scanlineSize);
	std::vector<uint8_t> uncompressed = NAMESPACE_PSAPI::RLE_Impl::DecompressPackBits<uint8_t>(compressed, data.size(), 1u);
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
	std::vector<uint8_t> compressed = NAMESPACE_PSAPI::RLE_Impl::CompressPackBits(std::span<uint8_t>(data.data(), data.size()), scanlineSize);

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

// Generates the worst case RLE scanline which will cause the most growth in size
void generateScanline(std::vector<uint8_t>& data, int width, int baseOffset) {
	for (int i = 0; i < width - 3; i += 3) {
		data[baseOffset + i] = 237;
		data[baseOffset + i + 1] = 237;
		data[baseOffset + i + 2] = 230;
	}
	size_t remainder = width % 3;
	if (remainder == 1) {
		data[baseOffset + width - 1] = 237;
	}
	else if (remainder > 0)
	{
		PSAPI_LOG_ERROR("Test", "The provided length will not produce a worst-case scenario for RLE!");
	}
}

TEST_CASE("Check RLE MaxCompressedSize")
{
	SUBCASE("PSD Call Packbits")
	{
		uint32_t width = 31;
		std::vector<uint8_t> data(width, 0);

		// Create a buffer with an alternating run/non-run pattern which is the worst-case for RLE
		generateScanline(data, width, 0);
		NAMESPACE_PSAPI::FileHeader header;
		header.m_Version = NAMESPACE_PSAPI::Enum::Version::Psd;
		size_t expectedSize = NAMESPACE_PSAPI::RLE_Impl::MaxCompressedSize<uint8_t>(header, 1, width, false);

		// Explicitly make the buffer larger than uncompressed data could ever be
		std::vector<uint8_t> buffer(data.size() * 2);
		auto compressedData = NAMESPACE_PSAPI::RLE_Impl::CompressPackBits(data, buffer);
		CHECK(expectedSize == compressedData.size());
	}
	SUBCASE("PSB Call Packbits")
	{
		uint32_t width = 31;
		std::vector<uint8_t> data(width, 0);

		// Create a buffer with an alternating run/non-run pattern which is the worst-case for RLE
		generateScanline(data, width, 0);

		NAMESPACE_PSAPI::FileHeader header;
		header.m_Version = NAMESPACE_PSAPI::Enum::Version::Psb;
		size_t expectedSize = NAMESPACE_PSAPI::RLE_Impl::MaxCompressedSize<uint8_t>(header, 1, width, false);

		// Explicitly make the buffer larger than uncompressed data could ever be
		std::vector<uint8_t> buffer(data.size() * 2);
		auto compressedData = NAMESPACE_PSAPI::RLE_Impl::CompressPackBits(data, buffer);
		CHECK(expectedSize == compressedData.size());
	}
	SUBCASE("PSD")
	{
		uint32_t width = 31;
		uint32_t height = 32;
		std::vector<uint8_t> data(width * height, 0);

		// Create a buffer with an alternating run/non-run pattern which is the worst-case for RLE
		for (int i = 0; i < height; ++i)
		{
			generateScanline(data, width, width * i);
		}
		NAMESPACE_PSAPI::FileHeader header;
		header.m_Version = NAMESPACE_PSAPI::Enum::Version::Psd;
		size_t expectedSize = NAMESPACE_PSAPI::RLE_Impl::MaxCompressedSize<uint8_t>(header, height, width);

		// Explicitly make the buffer larger than uncompressed data could ever be
		std::vector<uint8_t> buffer(data.size() * 2);
		auto resized_buffer = NAMESPACE_PSAPI::CompressRLE(std::span<uint8_t>(data), buffer, header, width, height);
		CHECK(expectedSize == resized_buffer.size());
	}
	SUBCASE("PSB")
	{
		uint32_t width = 31;
		uint32_t height = 32;
		std::vector<uint8_t> data(width * height, 0);

		// Create a buffer with an alternating run/non-run pattern which is the worst-case for RLE
		for (int i = 0; i < height; ++i)
		{
			generateScanline(data, width, width * i);
		}
		NAMESPACE_PSAPI::FileHeader header;
		header.m_Version = NAMESPACE_PSAPI::Enum::Version::Psb;
		size_t expectedSize = NAMESPACE_PSAPI::RLE_Impl::MaxCompressedSize<uint8_t>(header, height, width);

		// Explicitly make the buffer larger than uncompressed data could ever be
		std::vector<uint8_t> buffer(data.size() * 2);
		auto resized_buffer = NAMESPACE_PSAPI::CompressRLE(std::span<uint8_t>(data), buffer, header, width, height);
		CHECK(expectedSize == resized_buffer.size());
	}
}