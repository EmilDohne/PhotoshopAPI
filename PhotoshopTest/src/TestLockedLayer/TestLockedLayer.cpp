#include "doctest.h"

#include "../DetectArmMac.h"

#include "Macros.h"
#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/ImageLayer.h"
#include "LayeredFile/LayerTypes/GroupLayer.h"

#include <string>
#include <vector>



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Create File with locked layers and then check if we can read them again")
{
	using namespace NAMESPACE_PSAPI;
	using type = bpp16_t;
	constexpr int32_t width = 64;
	constexpr int32_t height = 64;
	constexpr int32_t size = width * height;

	{
		LayeredFile<type> file(Enum::ColorMode::RGB, width, height);
		{
			std::unordered_map<int16_t, std::vector<type>> data =
			{
				{0, std::vector<type>(size)},
				{1, std::vector<type>(size)},
				{2, std::vector<type>(size)},
			};
			auto params = typename Layer<type>::Params
			{
				.name = "Layer",
				.width = width,
				.height = height,
				.locked = true,
			};
			auto layer = std::make_shared<ImageLayer<type>>(std::move(data), params);
			file.addLayer(layer);
		}
		{
			auto params = typename Layer<type>::Params
			{
				.name = "Group",
				.locked = true
			};
			auto layer = std::make_shared<GroupLayer<type>>(params);
			file.addLayer(layer);
		}
		LayeredFile<type>::write(std::move(file), "LockedLayerFile.psb");
	}
	{
		auto file = LayeredFile<type>::read("LockedLayerFile.psb");
		for (const auto& layer : file.flatLayers())
		{
			CHECK(layer->m_IsLocked);
		}
	}
}
