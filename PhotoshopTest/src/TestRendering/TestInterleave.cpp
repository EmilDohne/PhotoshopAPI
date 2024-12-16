#include "doctest.h"

#include "PhotoshopAPI.h"
#include "../DetectArmMac.h"
#include "src/Core/Render/Interleave.h"

using namespace NAMESPACE_PSAPI;


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Interleave buffers preallocated buffer, variadic arguments")
{
	using type = uint16_t;

	std::vector<type> channel_r(10, 255);
	std::vector<type> channel_g(10, 100);
	std::vector<type> channel_b(10, 25);

	std::vector<type> buffer(30);

	Render::interleave(std::span<type>(buffer), std::span<type>(channel_r), std::span<type>(channel_g), std::span<type>(channel_b));

	CHECK(buffer[0] == 255);
	CHECK(buffer[1] == 100);
	CHECK(buffer[2] == 25);

	CHECK(buffer[27] == 255);
	CHECK(buffer[28] == 100);
	CHECK(buffer[29] == 25);
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#ifndef ARM_MAC_ARCH
	TEST_CASE("Interleave buffers preallocated buffer incorrect size, variadic arguments",
		* doctest::no_breaks(true)
		* doctest::no_output(true)
		* doctest::should_fail(true))
	{
		using type = uint16_t;

		std::vector<type> channel_r(10, 255);
		std::vector<type> channel_g(10, 100);
		std::vector<type> channel_b(10, 25);

		std::vector<type> buffer(25);

		// Should fail since buffer is too small
		Render::interleave(std::span<type>(buffer), std::span<type>(channel_r), std::span<type>(channel_g), std::span<type>(channel_b));
	}
#endif


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#ifndef ARM_MAC_ARCH
	TEST_CASE("Interleave buffers preallocated buffer mismatched sizes, variadic arguments",
		* doctest::no_breaks(true)
		* doctest::no_output(true)
		* doctest::should_fail(true))
	{
		using type = uint16_t;

		std::vector<type> channel_r(15, 255);
		std::vector<type> channel_g(10, 100);
		std::vector<type> channel_b(10, 25);

		std::vector<type> buffer(35);

		// Should fail since channel_r has the wrong size
		Render::interleave(std::span<type>(buffer), std::span<type>(channel_r), std::span<type>(channel_g), std::span<type>(channel_b));
	}
#endif

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Interleave buffers alloc, variadic arguments")
{
	using type = uint16_t;

	std::vector<type> channel_r(10, 255);
	std::vector<type> channel_g(10, 100);
	std::vector<type> channel_b(10, 25);

	auto result = Render::interleave_alloc(std::span<type>(channel_r), std::span<type>(channel_g), std::span<type>(channel_b));

	CHECK(result.size() == 30);

	CHECK(result[0] == 255);
	CHECK(result[1] == 100);
	CHECK(result[2] == 25);

	CHECK(result[27] == 255);
	CHECK(result[28] == 100);
	CHECK(result[29] == 25);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Interleave buffers alloc, vector span argument")
{
	using type = uint16_t;

	std::vector<type> channel_r(10, 255);
	std::vector<type> channel_g(10, 100);
	std::vector<type> channel_b(10, 25);

	std::vector<std::span<type>> channels = { std::span<type>(channel_r), std::span<type>(channel_g), std::span<type>(channel_b) };

	auto result = Render::interleave_alloc(channels);

	CHECK(result.size() == 30);

	CHECK(result[0] == 255);
	CHECK(result[1] == 100);
	CHECK(result[2] == 25);

	CHECK(result[27] == 255);
	CHECK(result[28] == 100);
	CHECK(result[29] == 25);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Interleave buffers preallocated buffer, vector span argument")
{
	using type = uint16_t;

	std::vector<type> channel_r(10, 255);
	std::vector<type> channel_g(10, 100);
	std::vector<type> channel_b(10, 25);

	std::vector<std::span<type>> channels = { std::span<type>(channel_r), std::span<type>(channel_g), std::span<type>(channel_b) };
	std::vector<type> buffer(30);

	Render::interleave(buffer, channels);

	CHECK(buffer[0] == 255);
	CHECK(buffer[1] == 100);
	CHECK(buffer[2] == 25);

	CHECK(buffer[27] == 255);
	CHECK(buffer[28] == 100);
	CHECK(buffer[29] == 25);
}