#include "doctest.h"

#include "Core/Struct/DescriptorStructure.h"


TEST_CASE("Test Descriptor Insertion")
{
	using namespace PhotoshopAPI;
	Descriptors::Descriptor descriptor{};

	descriptor.insert("key", 20.0f);

	CHECK(descriptor.contains("key"));
	CHECK(std::get<double>(descriptor.at("key")) == 20.0f);
	CHECK(std::holds_alternative<double>(descriptor["key"]));
}