#include "doctest.h"

#include "Macros.h"
#include "../TestMacros.h"
#include "Core/Compression/Compress_ZIP.h"
#include "Core/Compression/Decompress_ZIP.h"

#include <vector>
#include <limits>



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Prediction Roundtrip Flat Channel 8-bit")
{
	uint32_t width = 32;
	uint32_t height = 32;

	std::vector<uint8_t> channel(width * height, 255u);
	std::vector<uint8_t> buffer(width * height * sizeof(uint8_t), 0u);
	std::vector<uint8_t> dataExpected = channel;

	NAMESPACE_PSAPI::ZIP_Impl::PredictionEncode(channel, buffer, width, height);
	NAMESPACE_PSAPI::ZIP_Impl::RemovePredictionEncoding<uint8_t>(channel, width, height);

	CHECK(channel == dataExpected);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Prediction Roundtrip Large Channel 8-bit")
{
	uint32_t width = 256;
	uint32_t height = 256;

	std::vector<uint8_t> channel(width * height, 255u);
	std::vector<uint8_t> buffer(width * height * sizeof(uint8_t), 0u);
	std::vector<uint8_t> dataExpected = channel;

	NAMESPACE_PSAPI::ZIP_Impl::PredictionEncode(channel, buffer, width, height);
	NAMESPACE_PSAPI::ZIP_Impl::RemovePredictionEncoding<uint8_t>(channel, width, height);

	CHECK(channel == dataExpected);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Prediction Roundtrip Flat Channel 16-bit")
{
	uint32_t width = 32;
	uint32_t height = 32;

	std::vector<uint16_t> channel(width * height, 65535u);
	std::vector<uint8_t> buffer(width * height * sizeof(uint16_t), 0u);
	std::vector<uint16_t> dataExpected = channel;

	NAMESPACE_PSAPI::ZIP_Impl::PredictionEncode(channel, buffer, width, height);
	NAMESPACE_PSAPI::ZIP_Impl::RemovePredictionEncoding<uint16_t>(channel, width, height);

	CHECK(channel == dataExpected);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Prediction Roundtrip Large Channel 16-bit")
{
	uint32_t width = 256;
	uint32_t height = 256;

	std::vector<uint16_t> channel(width * height, 65535u);
	std::vector<uint8_t> buffer(width * height * sizeof(uint16_t), 0u);
	std::vector<uint16_t> dataExpected = channel;

	NAMESPACE_PSAPI::ZIP_Impl::PredictionEncode(channel, buffer, width, height);
	NAMESPACE_PSAPI::ZIP_Impl::RemovePredictionEncoding<uint16_t>(channel, width, height);

	CHECK(channel == dataExpected);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Prediction Roundtrip Flat Channel 32-bit")
{
	uint32_t width = 32;
	uint32_t height = 32;

	std::vector<float32_t> channel(width * height, 1.0f);
	std::vector<uint8_t> buffer(width * height * sizeof(float32_t), 0u);
	std::vector<float32_t> dataExpected = channel;

	NAMESPACE_PSAPI::ZIP_Impl::PredictionEncode(channel, buffer, width, height);
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			std::cout << "Y: " << y << "X:" << x << channel[y * width + x] << std::endl;
		}
	}
	NAMESPACE_PSAPI::ZIP_Impl::RemovePredictionEncoding<float32_t>(channel, width, height);

	CHECK_VEC_VERBOSE(channel, dataExpected);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Prediction Roundtrip Large Channel 32-bit")
{
	uint32_t width = 256;
	uint32_t height = 256;

	std::vector<float32_t> channel(width * height, 1.0f);
	std::vector<uint8_t> buffer(width * height * sizeof(float32_t), 0u);
	std::vector<float32_t> dataExpected = channel;

	NAMESPACE_PSAPI::ZIP_Impl::PredictionEncode(channel, buffer, width, height);
	NAMESPACE_PSAPI::ZIP_Impl::RemovePredictionEncoding<float32_t>(channel, width, height);

	CHECK_VEC_VERBOSE(channel, dataExpected);
}
