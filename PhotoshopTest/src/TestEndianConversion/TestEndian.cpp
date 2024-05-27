#include "doctest.h"

#include "../DetectArmMac.h"
#include "Core/Endian/EndianByteSwapArr.h"

#include <vector>


// Test 16-bit roundtripping
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Roundtripping 16-bit array")
{
	uint32_t width = 32;
	uint32_t height = 32;

	std::vector<NAMESPACE_PSAPI::bpp16_t> channel(width * height, 32768u);
	std::vector<NAMESPACE_PSAPI::bpp16_t> dataExpected = channel;	// We construct a copy here as EndianEncode will invalidate the data

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
	std::vector<NAMESPACE_PSAPI::bpp32_t> dataExpected = channel;	// We construct a copy here as EndianEncode will invalidate the data

	NAMESPACE_PSAPI::endianEncodeBEArray(channel);
	NAMESPACE_PSAPI::endianDecodeBEArray(channel);

	CHECK(dataExpected == channel);
}


// Test 16-bit large roundtripping
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Roundtripping 16-bit array large")
{
	uint32_t width = 2048;
	uint32_t height = 2048;

	std::vector<NAMESPACE_PSAPI::bpp16_t> channel(width * height, 32768u);
	std::vector<NAMESPACE_PSAPI::bpp16_t> dataExpected = channel;	// We construct a copy here as EndianEncode will invalidate the data

	NAMESPACE_PSAPI::endianEncodeBEArray(channel);
	NAMESPACE_PSAPI::endianDecodeBEArray(channel);

	CHECK(dataExpected == channel);
}


// Test 32-bit large roundtripping
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Roundtripping 32-bit array large")
{
	uint32_t width = 2048;
	uint32_t height = 2048;

	std::vector<NAMESPACE_PSAPI::bpp32_t> channel(width * height, 1.0f);
	std::vector<NAMESPACE_PSAPI::bpp32_t> dataExpected = channel;	// We construct a copy here as EndianEncode will invalidate the data

	NAMESPACE_PSAPI::endianEncodeBEArray(channel);
	NAMESPACE_PSAPI::endianDecodeBEArray(channel);

	CHECK(dataExpected == channel);
}


// Test 16-bit roundtripping uneven
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Roundtripping 16-bit array uneven")
{
	uint32_t width = 27;
	uint32_t height = 35;

	std::vector<NAMESPACE_PSAPI::bpp16_t> channel(width * height, 32768u);
	std::vector<NAMESPACE_PSAPI::bpp16_t> dataExpected = channel;	// We construct a copy here as EndianEncode will invalidate the data

	NAMESPACE_PSAPI::endianEncodeBEArray(channel);
	NAMESPACE_PSAPI::endianDecodeBEArray(channel);

	CHECK(dataExpected == channel);
}


// Test 32-bit roundtripping uneven
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Roundtripping 32-bit array uneven")
{
	uint32_t width = 27;
	uint32_t height = 35;

	std::vector<NAMESPACE_PSAPI::bpp32_t> channel(width * height, 1.0f);
	std::vector<NAMESPACE_PSAPI::bpp32_t> dataExpected = channel;	// We construct a copy here as EndianEncode will invalidate the data

	NAMESPACE_PSAPI::endianEncodeBEArray(channel);
	NAMESPACE_PSAPI::endianDecodeBEArray(channel);

	CHECK(dataExpected == channel);
}


// Test 16-bit large roundtripping uneven
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Roundtripping 16-bit array large uneven")
{
	uint32_t width = 3288;
	uint32_t height = 1671;

	std::vector<NAMESPACE_PSAPI::bpp16_t> channel(width * height, 32768u);
	std::vector<NAMESPACE_PSAPI::bpp16_t> dataExpected = channel;	// We construct a copy here as EndianEncode will invalidate the data

	NAMESPACE_PSAPI::endianEncodeBEArray(channel);
	NAMESPACE_PSAPI::endianDecodeBEArray(channel);

	CHECK(dataExpected == channel);
}


// Test 32-bit large roundtripping uneven
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Roundtripping 32-bit array large uneven")
{
	uint32_t width = 3288;
	uint32_t height = 1671;

	std::vector<NAMESPACE_PSAPI::bpp32_t> channel(width * height, 1.0f);
	std::vector<NAMESPACE_PSAPI::bpp32_t> dataExpected = channel;	// We construct a copy here as EndianEncode will invalidate the data

	NAMESPACE_PSAPI::endianEncodeBEArray(channel);
	NAMESPACE_PSAPI::endianDecodeBEArray(channel);

	CHECK(dataExpected == channel);
}


// Test 8-bit EndianDecodeBEBinaryArray
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Decode Binary 8-bit")
{
	PSAPI_LOG("Test", "Running Test: Endian Decode Binary 16-bit");
	uint32_t width = 32;
	uint32_t height = 32;

	std::vector<NAMESPACE_PSAPI::bpp8_t> binaryData(width * height, 128u);
	std::vector<NAMESPACE_PSAPI::bpp8_t> dataExpected = binaryData;

	auto result = NAMESPACE_PSAPI::endianDecodeBEBinaryArray<NAMESPACE_PSAPI::bpp8_t>(binaryData);

	CHECK(dataExpected == result);
}


// Test 16-bit EndianDecodeBEBinaryArray
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Decode Binary 16-bit")
{
	PSAPI_LOG("Test", "Running Test: Endian Decode Binary 16-bit");
	uint32_t width = 32;
	uint32_t height = 32;

	std::vector<NAMESPACE_PSAPI::bpp8_t> binaryData(width * height * sizeof(NAMESPACE_PSAPI::bpp16_t));
	std::vector<NAMESPACE_PSAPI::bpp16_t> dataExpected(width * height, 255u);
	for (int i = 0; i < binaryData.size(); ++i)
	{
		// Fill the data with 00FF which corresponds to 255 in big endian mode
		binaryData[i] = i % 2 == 0 ? 0 : 255;
	}

	auto result = NAMESPACE_PSAPI::endianDecodeBEBinaryArray<NAMESPACE_PSAPI::bpp16_t>(binaryData);

	CHECK(dataExpected.size() == result.size());
	CHECK(dataExpected == result);
}


// Test 32-bit EndianDecodeBEBinaryArray
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Decode Binary 32-bit")
{
	PSAPI_LOG("Test", "Running Test: Endian Decode Binary 32-bit");
	uint32_t width = 32;
	uint32_t height = 32;

	std::vector<NAMESPACE_PSAPI::bpp8_t> binaryData(width * height * sizeof(NAMESPACE_PSAPI::bpp32_t));
	std::vector<NAMESPACE_PSAPI::bpp32_t> dataExpected(width * height, 1.0f);
	for (int i = 0; i < binaryData.size() - 3; i += sizeof(NAMESPACE_PSAPI::bpp32_t))
	{
		// Fill the data with 3f800000 which corresponds to 1.0f in big endian mode
		binaryData[i] = 0x3f;
		binaryData[i + 1] = 0x80;
		binaryData[i + 2] = 0x00;
		binaryData[i + 3] = 0x00;
	}

	auto result = NAMESPACE_PSAPI::endianDecodeBEBinaryArray<NAMESPACE_PSAPI::bpp32_t>(binaryData);

	CHECK(dataExpected.size() == result.size());
	CHECK(dataExpected == result);
}


// Test 8-bit EndianDecodeBEBinaryArray uneven
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Decode Binary 8-bit uneven")
{
	PSAPI_LOG("Test", "Running Test: Endian Decode Binary 8-bit uneven");
	uint32_t width = 27;
	uint32_t height = 35;

	std::vector<NAMESPACE_PSAPI::bpp8_t> binaryData(width * height, 128u);
	std::vector<NAMESPACE_PSAPI::bpp8_t> dataExpected = binaryData;

	auto result = NAMESPACE_PSAPI::endianDecodeBEBinaryArray<NAMESPACE_PSAPI::bpp8_t>(binaryData);

	CHECK(dataExpected == result);
}


// Test 16-bit EndianDecodeBEBinaryArray uneven
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Decode Binary 16-bit uneven")
{
	PSAPI_LOG("Test", "Running Test: Endian Decode Binary 16-bit uneven");
	uint32_t width = 27;
	uint32_t height = 35;

	std::vector<NAMESPACE_PSAPI::bpp8_t> binaryData(width * height * sizeof(NAMESPACE_PSAPI::bpp16_t));
	std::vector<NAMESPACE_PSAPI::bpp16_t> dataExpected(width * height, 255u);
	for (int i = 0; i < binaryData.size(); ++i)
	{
		// Fill the data with 00FF which corresponds to 255 in big endian mode
		binaryData[i] = i % 2 == 0 ? 0 : 255;
	}

	auto result = NAMESPACE_PSAPI::endianDecodeBEBinaryArray<NAMESPACE_PSAPI::bpp16_t>(binaryData);

	CHECK(dataExpected.size() == result.size());
	CHECK(dataExpected == result);
}


// Test 32-bit EndianDecodeBEBinaryArray uneven
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Decode Binary 32-bit uneven")
{
	PSAPI_LOG("Test", "Running Test: Endian Decode Binary 32-bit uneven");
	uint32_t width = 27;
	uint32_t height = 35;

	std::vector<NAMESPACE_PSAPI::bpp8_t> binaryData(width * height * sizeof(NAMESPACE_PSAPI::bpp32_t));
	std::vector<NAMESPACE_PSAPI::bpp32_t> dataExpected(width * height, 1.0f);
	for (int i = 0; i < binaryData.size() - 3; i += sizeof(NAMESPACE_PSAPI::bpp32_t))
	{
		// Fill the data with 3f800000 which corresponds to 1.0f in big endian mode
		binaryData[i] = 0x3f;
		binaryData[i + 1] = 0x80;
		binaryData[i + 2] = 0x00;
		binaryData[i + 3] = 0x00;
	}

	auto result = NAMESPACE_PSAPI::endianDecodeBEBinaryArray<NAMESPACE_PSAPI::bpp32_t>(binaryData);

	CHECK(dataExpected.size() == result.size());
	CHECK(dataExpected == result);
}


// Test 8-bit EndianDecodeBEBinaryArray large
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Decode Binary 8-bit large")
{
	PSAPI_LOG("Test", "Running Test: Endian Decode Binary 8-bit large");
	uint32_t width = 2048;
	uint32_t height = 2048;

	std::vector<NAMESPACE_PSAPI::bpp8_t> binaryData(width * height, 128u);
	std::vector<NAMESPACE_PSAPI::bpp8_t> dataExpected = binaryData;

	auto result = NAMESPACE_PSAPI::endianDecodeBEBinaryArray<NAMESPACE_PSAPI::bpp8_t>(binaryData);

	CHECK(dataExpected == result);
}


// Test 16-bit EndianDecodeBEBinaryArray large
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Decode Binary 16-bit large")
{
	PSAPI_LOG("Test", "Running Test: Endian Decode Binary 16-bit large");
	uint32_t width = 2048;
	uint32_t height = 2048;

	std::vector<NAMESPACE_PSAPI::bpp8_t> binaryData(width * height * sizeof(NAMESPACE_PSAPI::bpp16_t));
	std::vector<NAMESPACE_PSAPI::bpp16_t> dataExpected(width * height, 255u);
	for (int i = 0; i < binaryData.size(); ++i)
	{
		// Fill the data with 00FF which corresponds to 255 in big endian mode
		binaryData[i] = i % 2 == 0 ? 0 : 255;
	}

	auto result = NAMESPACE_PSAPI::endianDecodeBEBinaryArray<NAMESPACE_PSAPI::bpp16_t>(binaryData);

	CHECK(dataExpected.size() == result.size());
	CHECK(dataExpected == result);
}


// Test 32-bit EndianDecodeBEBinaryArray large
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Decode Binary 32-bit large")
{
	PSAPI_LOG("Test", "Running Test: Endian Decode Binary 32-bit large");
	uint32_t width = 2048;
	uint32_t height = 2048;

	std::vector<NAMESPACE_PSAPI::bpp8_t> binaryData(width * height * sizeof(NAMESPACE_PSAPI::bpp32_t));
	std::vector<NAMESPACE_PSAPI::bpp32_t> dataExpected(width * height, 1.0f);
	for (int i = 0; i < binaryData.size() - 3; i += sizeof(NAMESPACE_PSAPI::bpp32_t))
	{
		// Fill the data with 3f800000 which corresponds to 1.0f in big endian mode
		binaryData[i] = 0x3f;
		binaryData[i + 1] = 0x80;
		binaryData[i + 2] = 0x00;
		binaryData[i + 3] = 0x00;
	}

	auto result = NAMESPACE_PSAPI::endianDecodeBEBinaryArray<NAMESPACE_PSAPI::bpp32_t>(binaryData);

	CHECK(dataExpected.size() == result.size());
	CHECK(dataExpected == result);
}



// Test 8-bit EndianDecodeBEBinaryArray large uneven
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Decode Binary 8-bit large uneven")
{
	PSAPI_LOG("Test", "Running Test: Endian Decode Binary 8-bit large uneven");
	uint32_t width = 3288;
	uint32_t height = 1671;

	std::vector<NAMESPACE_PSAPI::bpp8_t> binaryData(width * height, 128u);
	std::vector<NAMESPACE_PSAPI::bpp8_t> dataExpected = binaryData;

	auto result = NAMESPACE_PSAPI::endianDecodeBEBinaryArray<NAMESPACE_PSAPI::bpp8_t>(binaryData);

	CHECK(dataExpected == result);
}


// Test 16-bit EndianDecodeBEBinaryArray large uneven
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Decode Binary 16-bit large uneven")
{
	PSAPI_LOG("Test", "Running Test: Endian Decode Binary 16-bit large uneven");
	uint32_t width = 3288;
	uint32_t height = 1671;

	std::vector<NAMESPACE_PSAPI::bpp8_t> binaryData(width * height * sizeof(NAMESPACE_PSAPI::bpp16_t));
	std::vector<NAMESPACE_PSAPI::bpp16_t> dataExpected(width * height, 255u);
	for (int i = 0; i < binaryData.size(); ++i)
	{
		// Fill the data with 00FF which corresponds to 255 in big endian mode
		binaryData[i] = i % 2 == 0 ? 0 : 255;
	}

	auto result = NAMESPACE_PSAPI::endianDecodeBEBinaryArray<NAMESPACE_PSAPI::bpp16_t>(binaryData);

	CHECK(dataExpected.size() == result.size());
	CHECK(dataExpected == result);
}


// Test 32-bit EndianDecodeBEBinaryArray large uneven
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Endian Decode Binary 32-bit large uneven")
{
	PSAPI_LOG("Test", "Running Test: Endian Decode Binary 32-bit large uneven");
	uint32_t width = 3288;
	uint32_t height = 1671;

	std::vector<NAMESPACE_PSAPI::bpp8_t> binaryData(width * height * sizeof(NAMESPACE_PSAPI::bpp32_t));
	std::vector<NAMESPACE_PSAPI::bpp32_t> dataExpected(width * height, 1.0f);
	for (int i = 0; i < binaryData.size() - 3; i += sizeof(NAMESPACE_PSAPI::bpp32_t))
	{
		// Fill the data with 3f800000 which corresponds to 1.0f in big endian mode
		// used this tool to generate hex data https://www.h-schmidt.net/FloatConverter/IEEE754.html
		binaryData[i] = 0x3f;
		binaryData[i + 1] = 0x80;
		binaryData[i + 2] = 0x00;
		binaryData[i + 3] = 0x00;
	}

	auto result = NAMESPACE_PSAPI::endianDecodeBEBinaryArray<NAMESPACE_PSAPI::bpp32_t>(binaryData);

	CHECK(dataExpected.size() == result.size());
	CHECK(dataExpected == result);
}



// Test EndianDecodeBEBinaryArray incorrect data size 16-bit
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#ifndef ARM_MAC_ARCH
	TEST_CASE("Endian Decode Binary 16-bit incorrect data size" 
		* doctest::no_breaks(true)
		* doctest::no_output(true)
		* doctest::should_fail(true))
	{
		PSAPI_LOG("Test", "Running Test: Endian Decode Binary 16-bit incorrect data size");
		uint32_t width = 32;
		uint32_t height = 32;

		// The binary data in this case is on purpose an uneven number which should never work as 16 bits are 2
		// bytes and the size must be divisible by 2
		std::vector<NAMESPACE_PSAPI::bpp8_t> binaryData(width * height * sizeof(NAMESPACE_PSAPI::bpp16_t) - 1, 0);

		auto result = NAMESPACE_PSAPI::endianDecodeBEBinaryArray<NAMESPACE_PSAPI::bpp16_t>(binaryData);
	}
#endif

// Test EndianDecodeBEBinaryArray incorrect data size 32-bit
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#ifndef ARM_MAC_ARCH
	TEST_CASE("Endian Decode Binary 32-bit incorrect data size"
	* doctest::no_breaks(true)
	* doctest::no_output(true)
	* doctest::should_fail(true))
{
	PSAPI_LOG("Test", "Running Test: Endian Decode Binary 32-bit incorrect data size");
	uint32_t width = 32;
	uint32_t height = 32;

	// The binary data in this case is on purpose an uneven number which should never work as 16 bits are 2
	// bytes and the size must be divisible by 2
	std::vector<NAMESPACE_PSAPI::bpp8_t> binaryData(width * height * sizeof(NAMESPACE_PSAPI::bpp32_t) - 2, 0);

	auto result = NAMESPACE_PSAPI::endianDecodeBEBinaryArray<NAMESPACE_PSAPI::bpp32_t>(binaryData);
}
#endif