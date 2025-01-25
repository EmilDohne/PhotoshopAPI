#include "doctest.h"

#include "Core/Struct/DescriptorStructure.h"


TEST_CASE("Test Descriptor Insertion")
{
	using namespace PhotoshopAPI;
	Descriptors::Descriptor descriptor{};

	descriptor.insert("key", static_cast<double>(20.0f));

	CHECK(descriptor.contains("key"));
	CHECK(descriptor.at<double>("key") == 20.0f);
}