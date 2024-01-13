/*
Since we do not know which compression level etc. Photoshop uses for its zip compression we need to do the roundtripping slightly different.
Instead of reading the raw compressed bytes as we did with RLE we create an image and first compress, then uncompress it. Our goal is to make
sure these two results match.
*/
#include "doctest.h"

#include "Macros.h"
#include "Compression/ZIP.h"

#include <vector>
#include <limits>


// Check if the compression performs well on channels that are smaller than the buffer size specified in the Zip() function
// which is currently 16*1024 bytes
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Compress Flat Channel 16-bit")
{
	uint32_t width = 32;
	uint32_t height = 32;

	std::vector<uint16_t> channel(width * height, (std::numeric_limits<uint16_t>::max)());
	std::vector<uint16_t> dataExpected = channel;	// We construct a copy here as CompressZIP will invalidate the data

	std::vector<uint8_t> compressedData = NAMESPACE_PSAPI::CompressZIP<uint16_t>(channel, width, height);
	std::vector<uint16_t> uncompressedData = NAMESPACE_PSAPI::DecompressZIP<uint16_t>(compressedData, width, height);

	CHECK(dataExpected == uncompressedData);
}


// Check if the compression performs well on channels that are smaller than the buffer size specified in the Zip() function
// which is currently 16*1024 bytes
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Compress Flat Channel 32-bit")
{
	uint32_t width = 32;
	uint32_t height = 32;

	std::vector<float32_t> channel(width * height, 1.0f);
	std::vector<float32_t> dataExpected = channel;	// We construct a copy here as CompressZIP will invalidate the data

	std::vector<uint8_t> compressedData = NAMESPACE_PSAPI::CompressZIP<float32_t>(channel, width, height);
	std::vector<float32_t> uncompressedData = NAMESPACE_PSAPI::DecompressZIP<float32_t>(compressedData, width, height);

	CHECK(dataExpected == uncompressedData);
}


// Check if the compression performs well on channels that are bigger than the buffer size specified in the Zip() function
// which is currently 16*1024 bytes
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Compress Large Flat Channel 16-bit")
{
	uint32_t width = 256;
	uint32_t height = 256;

	std::vector<uint16_t> channel(width * height, (std::numeric_limits<uint16_t>::max)());
	std::vector<uint16_t> dataExpected = channel;	// We construct a copy here as CompressZIP will invalidate the data

	std::vector<uint8_t> compressedData = NAMESPACE_PSAPI::CompressZIP<uint16_t>(channel, width, height);
	std::vector<uint16_t> uncompressedData = NAMESPACE_PSAPI::DecompressZIP<uint16_t>(compressedData, width, height);

	CHECK(dataExpected == uncompressedData);
}


// Check if the compression performs well on channels that are bigger than the buffer size specified in the Zip() function
// which is currently 16*1024 bytes
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Compress Large Flat Channel 32-bit")
{
	uint32_t width = 256;
	uint32_t height = 256;

	std::vector<float32_t> channel(width * height, 1.0f);
	std::vector<float32_t> dataExpected = channel;	// We construct a copy here as CompressZIP will invalidate the data

	std::vector<uint8_t> compressedData = NAMESPACE_PSAPI::CompressZIP<float32_t>(channel, width, height);
	std::vector<float32_t> uncompressedData = NAMESPACE_PSAPI::DecompressZIP<float32_t>(compressedData, width, height);

	CHECK(dataExpected == uncompressedData);
}