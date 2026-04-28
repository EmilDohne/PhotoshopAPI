#include "doctest.h"

#include "Macros.h"
#include "LayeredFile/LayeredFile.h"

#include <vector>


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Blend fill round tripping")
{
	using namespace NAMESPACE_PSAPI;

	{
		auto file = LayeredFile<bpp8_t>::read("documents/BlendFill/blend_fill.psd");
		const auto layer_ptr = file.layers().at(0);
		// We choose fairly aggressive epsilon as this is internally represented by a
		CHECK(layer_ptr->fill() == doctest::Approx(0.51f).epsilon(1e-2));
		LayeredFile<bpp8_t>::write(std::move(file), "documents/out/blend_fill_out.psd");
	}

	{
		auto file = LayeredFile<bpp8_t>::read("documents/out/blend_fill_out.psd");
		const auto layer_ptr = file.layers().at(0);
		CHECK(layer_ptr->fill() == doctest::Approx(0.51f).epsilon(1e-2));
	}
}
