#include "doctest.h"

#include "Macros.h"
#include "Util/CoordinateUtil.h"
#include "PhotoshopFile/FileHeader.h"


TEST_CASE("Test Simple Extents")
{
	using namespace NAMESPACE_PSAPI;

	ChannelExtents extents(0, 0, 32, 32);

	SUBCASE("Roundtripping Extents")
	{
		ChannelCoordinates coords = generateChannelCoordinates(extents);
		ChannelExtents generatedExtents = generate_extents(coords);

		CHECK(generatedExtents.top == extents.top);
		CHECK(generatedExtents.left == extents.left);
		CHECK(generatedExtents.bottom == extents.bottom);
		CHECK(generatedExtents.right == extents.right);
	}

	SUBCASE("Matches Expectations")
	{
		ChannelCoordinates coords = generateChannelCoordinates(extents);
		CHECK(coords.centerX == 16);
		CHECK(coords.centerY == 16);
		CHECK(coords.width == 32);
		CHECK(coords.height == 32);
	}
}



TEST_CASE("Test Layer Smaller than document")
{
	using namespace NAMESPACE_PSAPI;

	ChannelExtents extents(8, 8, 24, 24);

	SUBCASE("Roundtripping Extents")
	{
		ChannelCoordinates coords = generateChannelCoordinates(extents);
		ChannelExtents generatedExtents = generate_extents(coords);

		CHECK(generatedExtents.top == extents.top);
		CHECK(generatedExtents.left == extents.left);
		CHECK(generatedExtents.bottom == extents.bottom);
		CHECK(generatedExtents.right == extents.right);
	}

	SUBCASE("Matches Expectations")
	{
		ChannelCoordinates coords = generateChannelCoordinates(extents);
		CHECK(coords.centerX == 16);
		CHECK(coords.centerY == 16);
		CHECK(coords.width == 16);
		CHECK(coords.height == 16);
	}
}


TEST_CASE("Test Layer not centered")
{
	using namespace NAMESPACE_PSAPI;

	ChannelExtents extents(0, 0, 24, 24);

	SUBCASE("Roundtripping Extents")
	{
		ChannelCoordinates coords = generateChannelCoordinates(extents);
		ChannelExtents generatedExtents = generate_extents(coords);

		CHECK(generatedExtents.top == extents.top);
		CHECK(generatedExtents.left == extents.left);
		CHECK(generatedExtents.bottom == extents.bottom);
		CHECK(generatedExtents.right == extents.right);
	}

	SUBCASE("Matches Expectations")
	{
		ChannelCoordinates coords = generateChannelCoordinates(extents);
		CHECK(coords.centerX == 12);
		CHECK(coords.centerY == 12);
		CHECK(coords.width == 24);
		CHECK(coords.height == 24);
	}
}


TEST_CASE("Test Layer out of bounds")
{
	using namespace NAMESPACE_PSAPI;

	ChannelExtents extents(16, 16, 48, 48);

	// Generate a simple header for a 32x32 document
	FileHeader header;
	header.m_Width = 32;
	header.m_Height = 32;

	SUBCASE("Roundtripping Extents")
	{
		ChannelCoordinates coords = generateChannelCoordinates(extents);
		ChannelExtents generatedExtents = generate_extents(coords);

		CHECK(generatedExtents.top == extents.top);
		CHECK(generatedExtents.left == extents.left);
		CHECK(generatedExtents.bottom == extents.bottom);
		CHECK(generatedExtents.right == extents.right);
	}

	SUBCASE("Matches Expectations")
	{
		ChannelCoordinates coords = generateChannelCoordinates(extents);
		CHECK(coords.centerX == 32);
		CHECK(coords.centerY == 32);
		CHECK(coords.width == 32);
		CHECK(coords.height == 32);
	}
}


TEST_CASE("Test Layer out of bounds one axis")
{
	using namespace NAMESPACE_PSAPI;

	ChannelExtents extents(0, 16, 32, 48);

	SUBCASE("Roundtripping Extents")
	{
		ChannelCoordinates coords = generateChannelCoordinates(extents);
		ChannelExtents generatedExtents = generate_extents(coords);

		CHECK(generatedExtents.top == extents.top);
		CHECK(generatedExtents.left == extents.left);
		CHECK(generatedExtents.bottom == extents.bottom);
		CHECK(generatedExtents.right == extents.right);
	}

	SUBCASE("Matches Expectations")
	{
		ChannelCoordinates coords = generateChannelCoordinates(extents);
		CHECK(coords.centerX == 32);
		CHECK(coords.centerY == 16);
		CHECK(coords.width == 32);
		CHECK(coords.height == 32);
	}
}


TEST_CASE("Test Layer uneven size")
{
	using namespace NAMESPACE_PSAPI;

	ChannelExtents extents(0, 0, 13, 16);
	SUBCASE("Roundtripping Extents")
	{
		ChannelCoordinates coords = generateChannelCoordinates(extents);
		ChannelExtents generatedExtents = generate_extents(coords);

		CHECK(generatedExtents.top == extents.top);
		CHECK(generatedExtents.left == extents.left);
		CHECK(generatedExtents.bottom == extents.bottom);
		CHECK(generatedExtents.right == extents.right);
	}

	SUBCASE("Matches Expectations")
	{
		ChannelCoordinates coords = generateChannelCoordinates(extents);
		CHECK(coords.centerX == 8.0f);
		CHECK(coords.centerY == 6.5f);
		CHECK(coords.width == 16);
		CHECK(coords.height == 13);
	}
}



TEST_CASE("Test Document uneven size")
{
	using namespace NAMESPACE_PSAPI;

	ChannelExtents extents(0, 0, 64, 64);

	SUBCASE("Roundtripping Extents")
	{
		ChannelCoordinates coords = generateChannelCoordinates(extents);
		ChannelExtents generatedExtents = generate_extents(coords);

		CHECK(generatedExtents.top == extents.top);
		CHECK(generatedExtents.left == extents.left);
		CHECK(generatedExtents.bottom == extents.bottom);
		CHECK(generatedExtents.right == extents.right);
	}

	SUBCASE("Matches Expectations")
	{
		ChannelCoordinates coords = generateChannelCoordinates(extents);
		CHECK(coords.centerX == 32);
		CHECK(coords.centerY == 32);
		CHECK(coords.width == 64);
		CHECK(coords.height == 64);
	}
}


TEST_CASE("Test Large size extents uneven")
{
	using namespace NAMESPACE_PSAPI;

	ChannelExtents extents(409, 21, 1600, 1138);

	SUBCASE("Roundtripping Extents")
	{
		ChannelCoordinates coords = generateChannelCoordinates(extents);
		ChannelExtents generatedExtents = generate_extents(coords);

		CHECK(generatedExtents.top == extents.top);
		CHECK(generatedExtents.left == extents.left);
		CHECK(generatedExtents.bottom == extents.bottom);
		CHECK(generatedExtents.right == extents.right);
	}

	SUBCASE("Matches Expectations")
	{
		ChannelCoordinates coords = generateChannelCoordinates(extents);
		CHECK(coords.centerX == 579.5f);
		CHECK(coords.centerY == 1004.5f);
		CHECK(coords.width == 1117);
		CHECK(coords.height == 1191);
	}
}


TEST_CASE("Test Large size full")
{
	using namespace NAMESPACE_PSAPI;

	ChannelExtents extents(0, 0, 2261, 4520);

	// Generate a simple header for a 32x32 document
	FileHeader header;
	header.m_Width = 4532;
	header.m_Height = 6000;

	SUBCASE("Roundtripping Extents")
	{
		ChannelCoordinates coords = generateChannelCoordinates(extents);
		ChannelExtents generatedExtents = generate_extents(coords);

		CHECK(generatedExtents.top == extents.top);
		CHECK(generatedExtents.left == extents.left);
		CHECK(generatedExtents.bottom == extents.bottom);
		CHECK(generatedExtents.right == extents.right);
	}
}