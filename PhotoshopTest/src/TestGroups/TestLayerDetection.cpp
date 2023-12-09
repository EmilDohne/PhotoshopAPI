#include "doctest.h"

#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/GroupLayer.h"

#include <variant>

TEST_CASE("Find layer in hierarchy")
{
	std::filesystem::path combined_path = std::filesystem::current_path();
	combined_path += "\\documents\\Groups\\Groups_8bit.psd";

	NAMESPACE_PSAPI::File file(combined_path);
	std::unique_ptr<NAMESPACE_PSAPI::PhotoshopFile> document = std::make_unique<NAMESPACE_PSAPI::PhotoshopFile>();
	bool didParse = document->read(file);
	REQUIRE(didParse);

	// Build our layeredFile
	NAMESPACE_PSAPI::LayeredFile<uint8_t> layeredFile(std::move(document));

	// Find a specific layer, check that it can be found and that it is of type GroupLayer
	const auto layerPtr = layeredFile.findLayer("GroupTopLevel/SingleNestedGroupedLayer/CollapsedGroup");
	REQUIRE(layerPtr);
	REQUIRE(dynamic_cast<NAMESPACE_PSAPI::GroupLayer<uint8_t>*>(layerPtr.get()));
}