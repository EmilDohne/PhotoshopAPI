#include "doctest.h"

#include "PhotoshopAPI.h"
#include "Core/Struct/DescriptorStructure.h"
#include "LayeredFile/LayerTypes/SmartObjectLayer.h"
#include "Core/Warp/SmartObjectWarp.h"

#include <memory>


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Create smartobject with path")
{
	using namespace NAMESPACE_PSAPI;
	using bpp_type = uint8_t;

	auto file = LayeredFile<bpp_type>(Enum::ColorMode::RGB, 64, 64);

	Layer<bpp_type>::Params lr_params{};
	lr_params.name = "SmartObject";
	lr_params.width = 64;
	lr_params.height = 32;

	auto layer = std::make_shared<SmartObjectLayer<bpp_type>>(file, lr_params, "documents/image_data/ImageStackerImage.jpg");
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("apply transformation to layer")
{
	using namespace NAMESPACE_PSAPI;
	using bpp_type = uint8_t;

	auto file = LayeredFile<bpp_type>(Enum::ColorMode::RGB, 64, 64);

	Layer<bpp_type>::Params lr_params{};
	lr_params.name = "SmartObject";
	lr_params.width = 64;
	lr_params.height = 32;

	auto layer = std::make_shared<SmartObjectLayer<bpp_type>>(file, lr_params, "documents/image_data/ImageStackerImage.jpg");
	layer->width(500);
	layer->height(250);


	CHECK(layer->width() == 500);
	CHECK(layer->height() == 250);
}