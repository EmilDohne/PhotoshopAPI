#include "doctest.h"

#include "PhotoshopAPI.h"
#include "Core/Geometry/Point.h"
#include "Core/Geometry/MeshOperations.h"


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Scale a quad using a homography")
{
	using namespace NAMESPACE_PSAPI;

	auto source_quad = Geometry::create_normalized_quad<double>();
	auto dest_quad   = Geometry::create_quad<double>(2.0f, 2.0f);

	auto source_quad_vec = std::vector<Geometry::Point2D<double>>(source_quad.begin(), source_quad.end());

	auto homography = Geometry::Operations::create_homography_matrix<double>(source_quad, dest_quad);
	Geometry::Operations::transform<double>(source_quad_vec, homography);
	for (size_t i = 0; i < 4; ++i)
	{
		CHECK(Geometry::Point2D<double>::equal(dest_quad[i], source_quad_vec[i]));
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Move a quad using a homography")
{
	using namespace NAMESPACE_PSAPI;

	auto source_quad = Geometry::create_normalized_quad<double>();
	auto dest_quad = Geometry::create_normalized_quad<double>();
	for (auto& pt : dest_quad)
	{
		pt.x += .5f;
	}

	auto source_quad_vec = std::vector<Geometry::Point2D<double>>(source_quad.begin(), source_quad.end());

	auto homography = Geometry::Operations::create_homography_matrix<double>(source_quad, dest_quad);
	Geometry::Operations::transform<double>(source_quad_vec, homography);
	for (size_t i = 0; i < 4; ++i)
	{
		CHECK(Geometry::Point2D<double>::equal(dest_quad[i], source_quad_vec[i]));
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Skew a quad using a homography")
{
	using namespace NAMESPACE_PSAPI;

	auto source_quad = Geometry::create_normalized_quad<double>();
	auto dest_quad = Geometry::create_normalized_quad<double>();
	dest_quad[0].x = .25f;
	dest_quad[1].x = 1.25f;

	auto source_quad_vec = std::vector<Geometry::Point2D<double>>(source_quad.begin(), source_quad.end());

	auto homography = Geometry::Operations::create_homography_matrix<double>(source_quad, dest_quad);
	Geometry::Operations::transform<double>(source_quad_vec, homography);
	for (size_t i = 0; i < 4; ++i)
	{
		CHECK(Geometry::Point2D<double>::equal(dest_quad[i], source_quad_vec[i]));
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Perspective transform a quad using a homography")
{
	using namespace NAMESPACE_PSAPI;

	auto source_quad = Geometry::create_normalized_quad<double>();
	auto dest_quad = Geometry::create_normalized_quad<double>();
	dest_quad[0].x = .25f;
	dest_quad[1].x = .75;


	auto source_quad_vec = std::vector<Geometry::Point2D<double>>(source_quad.begin(), source_quad.end());

	auto homography = Geometry::Operations::create_homography_matrix<double>(source_quad, dest_quad);
	Geometry::Operations::transform<double>(source_quad_vec, homography);
	for (size_t i = 0; i < 4; ++i)
	{
		CHECK(Geometry::Point2D<double>::equal(dest_quad[i], source_quad_vec[i]));
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TEST_CASE("Combined transform a quad using a homography")
{
	using namespace NAMESPACE_PSAPI;

	auto source_quad = Geometry::create_normalized_quad<double>();
	auto dest_quad = Geometry::create_normalized_quad<double>();
	// Perspective warp
	dest_quad[0].x = .25f;
	dest_quad[1].x = .75;
	// Move quad
	for (auto& pt : dest_quad)
	{
		pt.x += .5f;
	}

	auto source_quad_vec = std::vector<Geometry::Point2D<double>>(source_quad.begin(), source_quad.end());

	auto homography = Geometry::Operations::create_homography_matrix<double>(source_quad, dest_quad);
	Geometry::Operations::transform<double>(source_quad_vec, homography);
	for (size_t i = 0; i < 4; ++i)
	{
		CHECK(Geometry::Point2D<double>::equal(dest_quad[i], source_quad_vec[i]));
	}
}