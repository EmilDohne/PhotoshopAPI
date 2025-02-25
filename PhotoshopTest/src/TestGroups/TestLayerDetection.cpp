#include "doctest.h"

#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/GroupLayer.h"
#include "LayeredFile/LayerTypes/ImageLayer.h"

#include <variant>

// Test that the translation from PhotoshopFile -> LayeredFile work and that we can access our layers using path based searches

TEST_CASE("Find Image layer in hierarchy 8bit")
{
	std::filesystem::path combined_path = std::filesystem::current_path();
	combined_path += "/documents/Groups/Groups_8bit.psd";
	// Build our layeredFile
	NAMESPACE_PSAPI::LayeredFile<uint8_t> layeredFile = NAMESPACE_PSAPI::LayeredFile<uint8_t>::read(combined_path);

	// Find a specific layer, check that it can be found and that it is of type GroupLayer
	const std::shared_ptr<NAMESPACE_PSAPI::Layer<uint8_t>> layerPtr = layeredFile.find_layer("GroupTopLevel/CollapsedGroup/BlackLayer");
	REQUIRE(layerPtr);
	REQUIRE(std::dynamic_pointer_cast<NAMESPACE_PSAPI::ImageLayer<uint8_t>>(layerPtr));
}


TEST_CASE("Find Image layer in hierarchy 8bit PSB")
{
	std::filesystem::path combined_path = std::filesystem::current_path();
	combined_path += "/documents/Groups/Groups_8bit.psb";

	// Build our layeredFile
	NAMESPACE_PSAPI::LayeredFile<uint8_t> layeredFile = NAMESPACE_PSAPI::LayeredFile<uint8_t>::read(combined_path);

	// Find a specific layer, check that it can be found and that it is of type GroupLayer
	const std::shared_ptr<NAMESPACE_PSAPI::Layer<uint8_t>> layerPtr = layeredFile.find_layer("GroupTopLevel/CollapsedGroup/BlackLayer");
	REQUIRE(layerPtr);
	REQUIRE(std::dynamic_pointer_cast<NAMESPACE_PSAPI::ImageLayer<uint8_t>>(layerPtr));
}


TEST_CASE("Find Image layer in hierarchy 16bit")
{
	std::filesystem::path combined_path = std::filesystem::current_path();
	combined_path += "/documents/Groups/Groups_16bit.psd";

	// Build our layeredFile
	NAMESPACE_PSAPI::LayeredFile<uint16_t> layeredFile = NAMESPACE_PSAPI::LayeredFile<uint16_t>::read(combined_path);

	// Find a specific layer, check that it can be found and that it is of type GroupLayer
	const std::shared_ptr<NAMESPACE_PSAPI::Layer<uint16_t>> layerPtr = layeredFile.find_layer("GroupTopLevel/CollapsedGroup/BlackLayer");
	REQUIRE(layerPtr);
	REQUIRE(std::dynamic_pointer_cast<NAMESPACE_PSAPI::ImageLayer<uint16_t>>(layerPtr));
}


TEST_CASE("Find Image layer in hierarchy 32bit")
{
	std::filesystem::path combined_path = std::filesystem::current_path();
	combined_path += "/documents/Groups/Groups_32bit.psd";


	// Build our layeredFile
	NAMESPACE_PSAPI::LayeredFile<float32_t> layeredFile = NAMESPACE_PSAPI::LayeredFile<float32_t>::read(combined_path);

	// Find a specific layer, check that it can be found and that it is of type GroupLayer
	const std::shared_ptr<NAMESPACE_PSAPI::Layer<float32_t>> layerPtr = layeredFile.find_layer("GroupTopLevel/CollapsedGroup/BlackLayer");
	REQUIRE(layerPtr);
	REQUIRE(std::dynamic_pointer_cast<NAMESPACE_PSAPI::ImageLayer<float32_t>>(layerPtr));
}


TEST_CASE("Find Group layer in hierarchy")
{
	std::filesystem::path combined_path = std::filesystem::current_path();
	combined_path += "/documents/Groups/Groups_8bit.psd";

	// Build our layeredFile
	NAMESPACE_PSAPI::LayeredFile<uint8_t> layeredFile = NAMESPACE_PSAPI::LayeredFile<uint8_t>::read(combined_path);

	// Find a specific layer, check that it can be found and that it is of type GroupLayer
	const std::shared_ptr<NAMESPACE_PSAPI::Layer<uint8_t>> layerPtr = layeredFile.find_layer("GroupTopLevel/CollapsedGroup");
	REQUIRE(layerPtr);
	REQUIRE(std::dynamic_pointer_cast<NAMESPACE_PSAPI::GroupLayer<uint8_t>>(layerPtr));
}