#include "doctest.h"

#include "PhotoshopFile/PhotoshopFile.h"
#include "LayeredFile/LayeredFile.h"
#include "Macros.h"

#include <filesystem>



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Read Unicode layer name from psd file")
{
	using namespace NAMESPACE_PSAPI;

	std::filesystem::path psd_path = std::filesystem::current_path();
	psd_path += "/documents/UnicodeNames/UnicodeLayerNames.psd";

	LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read(psd_path);

	// Find the two layers by their names and check if the result is not null
	SUBCASE("Find overflow layer")
	{
		auto ptr = layeredFile.findLayer("äüöUnicodeNameOverflowPascalString--------------------------------------------------------------------------------------------------------------------");
		CHECK(ptr);
	}
	SUBCASE("Find unicode layer")
	{
		auto ptr = layeredFile.findLayer("UnicodeNameäää");
		CHECK(ptr);
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Read Unicode layer name from psb file")
{
	using namespace NAMESPACE_PSAPI;

	std::filesystem::path psb_path = std::filesystem::current_path();
	psb_path += "/documents/UnicodeNames/UnicodeLayerNames.psb";

	LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read(psb_path);

	// Find the two layers by their names and check if the result is not null
	SUBCASE("Find overflow layer")
	{
		auto ptr = layeredFile.findLayer("äüöUnicodeNameOverflowPascalString--------------------------------------------------------------------------------------------------------------------");
		CHECK(ptr);
	}
	SUBCASE("Find unicode layer")
	{
		auto ptr = layeredFile.findLayer("UnicodeNameäää");
		CHECK(ptr);
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Read write unicode layer name from psd file")
{
	using namespace NAMESPACE_PSAPI;

	std::filesystem::path psd_path = std::filesystem::current_path();
	psd_path += "/documents/UnicodeNames/UnicodeLayerNames.psd";

	{
		LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read(psd_path);
		LayeredFile<bpp8_t>::write(std::move(layeredFile), psd_path);
	}
	LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read(psd_path);


	// Find the two layers by their names and check if the result is not null
	SUBCASE("Find overflow layer")
	{
		auto ptr = layeredFile.findLayer("äüöUnicodeNameOverflowPascalString--------------------------------------------------------------------------------------------------------------------");
		CHECK(ptr);
	}
	SUBCASE("Find unicode layer")
	{
		auto ptr = layeredFile.findLayer("UnicodeNameäää");
		CHECK(ptr);
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Read write unicode layer name from psb file")
{
	using namespace NAMESPACE_PSAPI;

	std::filesystem::path psb_path = std::filesystem::current_path();
	psb_path += "/documents/UnicodeNames/UnicodeLayerNames.psb";

	{
		LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read(psb_path);
		LayeredFile<bpp8_t>::write(std::move(layeredFile), psb_path);
	}
	LayeredFile<bpp8_t> layeredFile = LayeredFile<bpp8_t>::read(psb_path);


	// Find the two layers by their names and check if the result is not null
	SUBCASE("Find overflow layer")
	{
		auto ptr = layeredFile.findLayer("äüöUnicodeNameOverflowPascalString--------------------------------------------------------------------------------------------------------------------");
		CHECK(ptr);
	}
	SUBCASE("Find unicode layer")
	{
		auto ptr = layeredFile.findLayer("UnicodeNameäää");
		CHECK(ptr);
	}
}