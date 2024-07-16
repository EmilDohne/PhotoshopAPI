#include "doctest.h"

#include "Core/Struct/DescriptorStructure.h"


TEST_CASE("Parse Descriptor data from binary vec")
{
	// This data was generate by hexdumping a Smart object layer tagged block
	auto binaryFile = PhotoshopAPI::File("documents/binary_data/Descriptor/placed_layer_taggedblock.bin");
	auto descriptor = PhotoshopAPI::Descriptor{};

	descriptor.read(binaryFile);
	PSAPI_LOG("", "");
}