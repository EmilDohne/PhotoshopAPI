#include "doctest.h"

#include "Util/Endian/EndianByteSwapArr.h"

#include <vector>


// Test 16-bit roundtripping
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Roundtripping 16-bit array")
{
	uint32_t width = 32;
	uint32_t height = 32;

	std::vector<NAMESPACE_PSAPI::bpp16_t> channel(width * height, 1.0f);
	std::vector<NAMESPACE_PSAPI::bpp16_t> dataExpected = channel;	// We construct a copy here as CompressZIP will invalidate the data

	NAMESPACE_PSAPI::endianEncodeBEArray(channel);
	NAMESPACE_PSAPI::endianDecodeBEArray(channel);

	CHECK(dataExpected == channel);
}


// Test 32-bit roundtripping
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Roundtripping 32-bit array")
{
	uint32_t width = 32;
	uint32_t height = 32;

	std::vector<NAMESPACE_PSAPI::bpp32_t> channel(width * height, 1.0f);
	std::vector<NAMESPACE_PSAPI::bpp32_t> dataExpected = channel;	// We construct a copy here as CompressZIP will invalidate the data

	NAMESPACE_PSAPI::endianEncodeBEArray(channel);
	NAMESPACE_PSAPI::endianDecodeBEArray(channel);

	CHECK(dataExpected == channel);
}


// Test 16-bit large roundtripping
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Roundtripping 16-bit array")
{
	uint32_t width = 1024;
	uint32_t height = 1024;

	std::vector<NAMESPACE_PSAPI::bpp16_t> channel(width * height, 1.0f);
	std::vector<NAMESPACE_PSAPI::bpp16_t> dataExpected = channel;	// We construct a copy here as CompressZIP will invalidate the data

	NAMESPACE_PSAPI::endianEncodeBEArray(channel);
	NAMESPACE_PSAPI::endianDecodeBEArray(channel);

	CHECK(dataExpected == channel);
}


// Test 32-bit large roundtripping
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Roundtripping 32-bit array")
{
	uint32_t width = 1024;
	uint32_t height = 1024;

	std::vector<NAMESPACE_PSAPI::bpp32_t> channel(width * height, 1.0f);
	std::vector<NAMESPACE_PSAPI::bpp32_t> dataExpected = channel;	// We construct a copy here as CompressZIP will invalidate the data

	NAMESPACE_PSAPI::endianEncodeBEArray(channel);
	NAMESPACE_PSAPI::endianDecodeBEArray(channel);

	CHECK(dataExpected == channel);
}