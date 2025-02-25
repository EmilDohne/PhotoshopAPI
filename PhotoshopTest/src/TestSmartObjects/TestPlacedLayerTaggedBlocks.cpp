#include "doctest.h"

#include "Core/Struct/DescriptorStructure.h"
#include "LayeredFile/LayerTypes/SmartObjectLayer.h"
#include "Core/Warp/SmartObjectWarp.h"

#include <fstream>
#include <algorithm>


/// Adopted from: https://stackoverflow.com/a/39097160
bool compare_files(const std::filesystem::path path_1, const std::filesystem::path path_2)
{
	std::ifstream file_1(path_1, std::ifstream::ate | std::ifstream::binary);
	std::ifstream file_2(path_2, std::ifstream::ate | std::ifstream::binary);

	const auto size_1 = file_1.tellg();
	const auto size_2 = file_2.tellg();
	const auto min_size = std::min(size_1, size_2);

	file_1.seekg(0);
	file_2.seekg(0);

	std::vector<char> data_1(min_size);
	std::vector<char> data_2(min_size);

	file_1.read(data_1.data(), data_1.size());
	file_2.read(data_2.data(), data_2.size());

	for (size_t i = 0; i < min_size; ++i)
	{
		auto res = data_1[i] == data_2[i];
		if (!res)
		{
			PSAPI_LOG("Test", 
				"binary data is mismatched at index %zu, expected {%d} got {%d}", 
				i, static_cast<int>(data_1[i]), static_cast<int>(data_2[i]));
			return res;
		}
	}

	// Check that if there is padding bytes these are all 0.
	if (size_1 > min_size)
	{
		std::vector<uint8_t> padding(size_1 - min_size);
		file_1.read(reinterpret_cast<char*>(padding.data()), padding.size());
		return std::all_of(padding.begin(), padding.end(), [](uint8_t v) { return v == 0; });
	}
	if (size_2 > min_size)
	{
		std::vector<uint8_t> padding(size_2 - min_size);
		file_2.read(reinterpret_cast<char*>(padding.data()), padding.size());
		return std::all_of(padding.begin(), padding.end(), [](uint8_t v) { return v == 0; });
	}

	return true;
}


/// Check a descriptor for read, write and parity on roundtripping
void check_descriptor(std::filesystem::path filepath, std::filesystem::path outpath)
{
	using namespace NAMESPACE_PSAPI;
	Descriptors::Descriptor placed_layer_descriptor;
	SUBCASE("Read Write Descriptor")
	{
		File descriptor_file(filepath);

		File::FileParams out_params;
		out_params.doRead = false;
		out_params.forceOverwrite = true;
		File descriptor_out(outpath, out_params);

		placed_layer_descriptor.read(descriptor_file);
		placed_layer_descriptor.write(descriptor_out);
	}
	SUBCASE("Check Struct Equality")
	{
		File descriptor_file(filepath);
		placed_layer_descriptor.read(descriptor_file);

		Descriptors::Descriptor tmp_descriptor;
		File tmp_file(outpath);
		tmp_descriptor.read(tmp_file);

		CHECK(placed_layer_descriptor == tmp_descriptor);
	}
	SUBCASE("Check Binary Equality")
	{
		CHECK(compare_files(filepath, outpath));
	}
}



TEST_CASE("Read Distort Warp Descriptor")
{
	check_descriptor(
		"documents/binary_data/Descriptor/DistortWarp_PlacedLayerBlock.bin", 
		"documents/binary_data/Descriptor/DistortWarp_PlacedLayerBlock_out.bin"
	);
}


TEST_CASE("Read FX Perspective Warp Descriptor")
{
	check_descriptor(
		"documents/binary_data/Descriptor/FXPerspectiveWarp_PlacedLayerBlock.bin",
		"documents/binary_data/Descriptor/FXPerspectiveWarp_PlacedLayerBlock_out.bin"
	);
}


TEST_CASE("Read FX Puppet Warp Descriptor")
{
	check_descriptor(
		"documents/binary_data/Descriptor/FXPuppetWarp_PlacedLayerBlock.bin",
		"documents/binary_data/Descriptor/FXPuppetWarp_PlacedLayerBlock_out.bin"
	);
}


TEST_CASE("Read Perspective Warp Descriptor")
{
	check_descriptor(
		"documents/binary_data/Descriptor/PerspectiveWarp_PlacedLayerBlock.bin",
		"documents/binary_data/Descriptor/PerspectiveWarp_PlacedLayerBlock_out.bin"
	);
}


TEST_CASE("Read Quilt Warp Descriptor")
{
	check_descriptor(
		"documents/binary_data/Descriptor/QuiltWarp_PlacedLayerBlock.bin",
		"documents/binary_data/Descriptor/QuiltWarp_PlacedLayerBlock_out.bin"
	);
}


TEST_CASE("Read Skew Warp Descriptor")
{
	check_descriptor(
		"documents/binary_data/Descriptor/SkewWarp_PlacedLayerBlock.bin",
		"documents/binary_data/Descriptor/SkewWarp_PlacedLayerBlock_out.bin"
	);
}


TEST_CASE("Read Warp Descriptor")
{
	check_descriptor(
		"documents/binary_data/Descriptor/Warp_PlacedLayerBlock.bin",
		"documents/binary_data/Descriptor/Warp_PlacedLayerBlock_out.bin"
	);
}