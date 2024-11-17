#include "doctest.h"

#include "Core/Struct/DescriptorStructure.h"
#include "LayeredFile/LayerTypes/SmartObjectLayer.h"
#include "Core/Warp/SmartObjectWarp.h"



TEST_CASE("Create smartobject with path")
{
	using namespace NAMESPACE_PSAPI;

	auto file = LayeredFile<uint8_t>(Enum::ColorMode::RGB, 64, 64);

	Layer::Params lr_params{};
	lr_params.name = "SmartObject";
	lr_params.width = 64;
	lr_params.height = 32;

	auto layer = std::make_shared<SmartObjectLayer>(file, lr_params, "documents/image_data/ImageStackerImage.jpg");
}