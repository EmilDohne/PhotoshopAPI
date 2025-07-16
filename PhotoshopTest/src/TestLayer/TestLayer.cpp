#include "doctest.h"


#include "Macros.h"
#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/ImageLayer.h"
#include "LayeredFile/LayerTypes/GroupLayer.h"

#include <string>
#include <vector>


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Read clipping masks")
{
	using namespace NAMESPACE_PSAPI;

	auto document = LayeredFile<bpp8_t>::read("documents/ClippingMasks/clipping_masks.psd");

	auto layer_clipped_toplevel = find_layer_as<bpp8_t, ImageLayer>("clipping_toplevel", document);
	auto layer_clipped_nested = find_layer_as<bpp8_t, ImageLayer>("group/clipping_nested", document);

	SUBCASE("read data")
	{
		CHECK(layer_clipped_toplevel->clipping_mask());
		CHECK(layer_clipped_nested->clipping_mask());
	}

	SUBCASE("set on group layer")
	{
		auto layer_group = find_layer_as<bpp8_t, GroupLayer>("group", document);
		layer_group->clipping_mask(true);
		LayeredFile<bpp8_t>::write(std::move(document), "documents/clipping_mask_invalid_layer_1.psd");
	}

	SUBCASE("set on lowest level of group")
	{
		auto document_2 = LayeredFile<bpp8_t>::read("documents/ClippingMasks/clipping_masks.psd");
		auto layer_nested_bottom = find_layer_as<bpp8_t, ImageLayer>("group/Layer 3", document_2);

		layer_nested_bottom->clipping_mask(true);
		
		LayeredFile<bpp8_t>::write(std::move(document_2), "documents/clipping_mask_invalid_layer_2.psd");
	}

	SUBCASE("set on lowest level of file")
	{
		auto document_3 = LayeredFile<bpp8_t>::read("documents/ClippingMasks/clipping_masks.psd");
		auto layer_bottom = find_layer_as<bpp8_t, ImageLayer>("Layer 0", document_3);

		layer_bottom->clipping_mask(true);

		LayeredFile<bpp8_t>::write(std::move(document_3), "documents/clipping_mask_invalid_layer_3.psd");
	}
}