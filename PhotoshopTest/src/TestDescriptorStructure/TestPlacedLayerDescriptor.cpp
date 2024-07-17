#include "doctest.h"

#include "Core/Struct/DescriptorStructure.h"
#include "../TestMacros.h"


// The data path all test cases in this file must use
static const std::string dataPath = "documents/binary_data/Descriptor/placed_layer_taggedblock.bin";
static const std::string dataPathOut = "documents/binary_data/Descriptor/placed_layer_taggedblock_modified.bin";


// Simply check that the read doesnt fail without any other checks
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Parse Descriptor data from binary vec")
{
	// This data was generate by hexdumping a Smart object layer tagged block
	auto binaryFile = PhotoshopAPI::File(dataPath);
	auto descriptor = PhotoshopAPI::Descriptors::Descriptor{};

	descriptor.read(binaryFile);
}


// read-write data parity check
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Parse Descriptor data from binary vec")
{
	// This data was generate by hexdumping a Smart object layer tagged block
	auto binaryFile = PhotoshopAPI::File(dataPath);
	auto fileDataOrig = ReadBinaryArray<uint8_t>(binaryFile, binaryFile.getSize());
	binaryFile.setOffset(0u);

	// Dump the file to disk
	{
		auto descriptor = PhotoshopAPI::Descriptors::Descriptor{};
		descriptor.read(binaryFile);
		PhotoshopAPI::File::FileParams params{};
		params.doRead = false;
		params.forceOverwrite = true;
		auto binaryFileOut = PhotoshopAPI::File(dataPathOut, params);
		descriptor.write(binaryFileOut);
	}

	auto binaryFileRoundtripped = PhotoshopAPI::File(dataPathOut);
	auto fileDataParsed = ReadBinaryArray<uint8_t>(binaryFileRoundtripped, binaryFileRoundtripped.getSize());

	CHECK(fileDataOrig.size() == fileDataParsed.size());
	CHECK_VEC_VERBOSE(fileDataOrig, fileDataParsed);

}