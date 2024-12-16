#include "doctest.h"

#include "PhotoshopAPI.h"
#include "../DetectArmMac.h"
#include "src/Core/Render/Interleave.h"

#include <algorithm>

using namespace NAMESPACE_PSAPI;




// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Deinterleave buffers preallocated buffer, variadic span argument")
{
	using type = uint16_t;

	std::vector<type> channel_r(3);
	std::vector<type> channel_g(3);
	std::vector<type> channel_b(3);

	std::vector<type> buffer =
	{
		255, 100, 25,
		255, 100, 25,
		255, 100, 25,
	};

	Render::deinterleave(std::span<const type>(buffer), std::span<type>(channel_r), std::span<type>(channel_g), std::span<type>(channel_b));

	CHECK(std::all_of(channel_r.begin(), channel_r.end(), [](type value) { return value == 255; }));
	CHECK(std::all_of(channel_g.begin(), channel_g.end(), [](type value) { return value == 100; }));
	CHECK(std::all_of(channel_b.begin(), channel_b.end(), [](type value) { return value == 25; }));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#ifndef ARM_MAC_ARCH
	TEST_CASE("Deinterleave buffers preallocated buffer, variadic span argument, mismatched span size",
		* doctest::no_breaks(true)
		* doctest::no_output(true)
		* doctest::should_fail(true))
	{
		using type = uint16_t;

		std::vector<type> channel_r(4);
		std::vector<type> channel_g(3);
		std::vector<type> channel_b(3);

		std::vector<type> buffer =
		{
			255, 100, 25,
			255, 100, 25,
			255, 100, 25,
		};

		Render::deinterleave(std::span<const type>(buffer), std::span<type>(channel_r), std::span<type>(channel_g), std::span<type>(channel_b));
	}
#endif


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#ifndef ARM_MAC_ARCH
	TEST_CASE("Deinterleave buffers preallocated buffer, variadic span argument, incorrect buffer size",
		*doctest::no_breaks(true)
		* doctest::no_output(true)
		* doctest::should_fail(true))
	{
		using type = uint16_t;

		std::vector<type> channel_r(3);
		std::vector<type> channel_g(3);
		std::vector<type> channel_b(3);

		std::vector<type> buffer =
		{
			255, 100, 25,
			255, 100, 25,
			255, 100, 25,
			50
		};

		Render::deinterleave(std::span<const type>(buffer), std::span<type>(channel_r), std::span<type>(channel_g), std::span<type>(channel_b));
	}
#endif



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Deinterleave buffers allocate buffer")
{
	using type = uint16_t;

	std::vector<type> buffer =
	{
		255, 100, 25,
		255, 100, 25,
		255, 100, 25,
	};

	// deinterleave into 3 different channels
	auto result = Render::deinterleave_alloc(std::span<const type>(buffer), 3);

	CHECK(std::all_of(result[0].begin(), result[0].end(), [](type value) { return value == 255; }));
	CHECK(std::all_of(result[1].begin(), result[1].end(), [](type value) { return value == 100; }));
	CHECK(std::all_of(result[2].begin(), result[2].end(), [](type value) { return value == 25; }));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#ifndef ARM_MAC_ARCH
TEST_CASE("Deinterleave buffers allocate buffer, incorrect num channels",
	*doctest::no_breaks(true)
	* doctest::no_output(true)
	* doctest::should_fail(true))
{
	using type = uint16_t;

	std::vector<type> buffer =
	{
		255, 100, 25,
		255, 100, 25,
		255, 100, 25,
	};

	// deinterleave into 4 different channels, should fail as its not divisible by 4 cleanly
	auto result = Render::deinterleave_alloc(std::span<const type>(buffer), 4);
}
#endif



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Deinterleave buffers preallocated spans")
{
	using type = uint16_t;

	std::vector<type> channel_r(3);
	std::vector<type> channel_g(3);
	std::vector<type> channel_b(3);
	std::vector<std::span<type>> channel_spans = { std::span<type>(channel_r), std::span<type>(channel_g), std::span<type>(channel_b) };

	std::vector<type> buffer =
	{
		255, 100, 25,
		255, 100, 25,
		255, 100, 25,
	};

	// deinterleave into 3 different channels
	Render::deinterleave(std::span<const type>(buffer), channel_spans);

	CHECK(std::all_of(channel_r.begin(), channel_r.end(), [](type value) { return value == 255; }));
	CHECK(std::all_of(channel_g.begin(), channel_g.end(), [](type value) { return value == 100; }));
	CHECK(std::all_of(channel_b.begin(), channel_b.end(), [](type value) { return value == 25; }));
}