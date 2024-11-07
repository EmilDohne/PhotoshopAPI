#include "SmartObjectWarp.h"


#include <OpenImageIO/imageio.h>

#include <memory>

PSAPI_NAMESPACE_BEGIN

namespace SmartObject
{

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Descriptors::Descriptor Warp::serialize() const
	{
		throw std::runtime_error("Unimplemented");
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Warp Warp::deserialize(const Descriptors::Descriptor& warp_descriptor, const Descriptors::List& transform, const Descriptors::List& non_affine_transform, normal_warp)
	{
		Warp warp;
		warp.warp_type(WarpType::normal);
		try
		{
			// Retrieve bounds descriptor (nested Descriptor)
			const auto& boundsDescriptor = warp_descriptor.at<Descriptors::Descriptor>("bounds");
			warp.m_Bounds[0] = boundsDescriptor.at<double>("Top ");
			warp.m_Bounds[1] = boundsDescriptor.at<double>("Left");
			warp.m_Bounds[2] = boundsDescriptor.at<double>("Btom");
			warp.m_Bounds[3] = boundsDescriptor.at<double>("Rght");

			// Retrieve customEnvelopeWarp descriptor (nested Descriptor)
			const auto& customEnvelopeWarp = warp_descriptor.at<Descriptors::Descriptor>("customEnvelopeWarp");
			const auto& meshPoints = customEnvelopeWarp.at<Descriptors::ObjectArray>("meshPoints");

			// Retrieve Hrzn and Vrtc within meshPoints (UnitFloats)
			const auto& hrznValues = meshPoints.at<Descriptors::UnitFloats>("Hrzn").m_Values;
			const auto& vrtcValues = meshPoints.at<Descriptors::UnitFloats>("Vrtc").m_Values;

			if (hrznValues.size() != vrtcValues.size())
			{
				PSAPI_LOG_ERROR("SmartObjectWarp",
					"Expected horizontal and vertical points to have the same size, instead got {%zu, %zu}",
					hrznValues.size(), vrtcValues.size());
			}
			if (hrznValues.size() != 16)
			{
				PSAPI_LOG_ERROR("SmartObjectWarp",
					"Expected horizontal and vertical points to have 16 elements, instead got %zu",
					hrznValues.size());
			}

			warp.m_WarpPoints.reserve(hrznValues.size());
			for (size_t i = 0; i < hrznValues.size(); ++i)
			{
				warp.m_WarpPoints.push_back(Geometry::Point2D<double>(hrznValues[i], vrtcValues[i]));
			}
		}
		catch (const std::runtime_error& e)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Internal Error: Invalid descriptor type encountered. Full exception: %s", e.what());
		}
		catch (const std::out_of_range& e)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Internal Error: Invalid descriptor key encountered. Full exception: %s", e.what());
		}

		// Deserialize the common descriptor keys between quilt and normal warp
		Warp::deserializeCommon(warp, warp_descriptor);
		warp.non_affine_mesh(Warp::generate_non_affine_mesh(transform, non_affine_transform));

		return warp;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Warp Warp::deserialize(const Descriptors::Descriptor& quilt_warp_descriptor, const Descriptors::List& transform, const Descriptors::List& non_affine_transform, quilt_warp)
	{
		Warp warp;
		warp.warp_type(WarpType::quilt);

		try
		{
			// Retrieve bounds descriptor (nested Descriptor)
			const auto& boundsDescriptor = quilt_warp_descriptor.at<Descriptors::Descriptor>("bounds");
			[[maybe_unused]] const auto& top = boundsDescriptor.at<double>("Top ");
			[[maybe_unused]] const auto& left = boundsDescriptor.at<double>("Left");
			[[maybe_unused]] const auto& bottom = boundsDescriptor.at<double>("Btom");
			[[maybe_unused]] const auto& right = boundsDescriptor.at<double>("Rght");

			// Retrieve deformNumRows and deformNumCols (int32_t)
			const auto& deformNumRows = quilt_warp_descriptor.at<int32_t>("deformNumRows");
			const auto& deformNumCols = quilt_warp_descriptor.at<int32_t>("deformNumCols");

			// Retrieve customEnvelopeWarp descriptor (nested Descriptor)
			const auto& customEnvelopeWarp = quilt_warp_descriptor.at<Descriptors::Descriptor>("customEnvelopeWarp");
			const auto& meshPoints = customEnvelopeWarp.at<Descriptors::ObjectArray>("meshPoints");

			// Retrieve Hrzn and Vrtc within meshPoints (UnitFloats)
			const auto& hrznValues = meshPoints.at<Descriptors::UnitFloats>("Hrzn").m_Values;
			const auto& vrtcValues = meshPoints.at<Descriptors::UnitFloats>("Vrtc").m_Values;

			if (hrznValues.size() != vrtcValues.size())
			{
				PSAPI_LOG_ERROR("SmartObjectWarp", "Internal Error: number of horizontal and vertical points is not identical");
			}
			if (hrznValues.size() != static_cast<size_t>(deformNumRows * deformNumCols))
			{
				PSAPI_LOG_ERROR("SmartObjectWarp", "Internal Error: number of horizontal and vertical points does not match expected number of pts");
			}
			std::vector<Geometry::Point2D<double>> warp_points;
			for (size_t i = 0; i < hrznValues.size(); ++i)
			{
				warp_points.push_back({ hrznValues[i], vrtcValues[i] });
			}
			warp = Warp(warp_points, deformNumCols, deformNumRows);


			// Retrieve the quilt slices over x and y, these are probably the actual slice locations?
			// They are for some reason nested two levels deep
			const auto& quiltSlicesX = customEnvelopeWarp.at<Descriptors::ObjectArray>("quiltSliceX").at<Descriptors::UnitFloats>("quiltSliceX");
			const auto& quiltSlicesY = customEnvelopeWarp.at<Descriptors::ObjectArray>("quiltSliceY").at<Descriptors::UnitFloats>("quiltSliceY");

			warp.quilt_slices_x(quiltSlicesX.m_Values);
			warp.quilt_slices_x(quiltSlicesY.m_Values);
		}
		catch (const std::runtime_error& e)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Internal Error: Invalid descriptor type encountered. Full exception: %s", e.what());
		}
		catch (const std::out_of_range& e)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Internal Error: Invalid descriptor key encountered. Full exception: %s", e.what());
		}

		Warp::deserializeCommon(warp, quilt_warp_descriptor);
		warp.non_affine_mesh(Warp::generate_non_affine_mesh(transform, non_affine_transform));

		return warp;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	std::array<Geometry::Point2D<double>, 4> Warp::generate_non_affine_mesh(const Descriptors::List& transform, const Descriptors::List& non_affine_transform)
	{
		std::vector<double> transformItems = transform.as<double>();
		std::vector<double> nonAffineTransformItems = non_affine_transform.as<double>();
		if (transformItems.size() != nonAffineTransformItems.size())
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Invalid transform and non-affine transform encountered, expected both to be of exactly the same size");
		}
		if (transformItems.size() != 8)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Invalid transform and non-affine transform encountered, expected both to be of size 8, instead got %zu", transformItems.size());
		}

		// Convert to mesh so we can easily apply our transformation
		Geometry::Mesh<double> nonAffineTransformMesh;
		{
			std::vector<Geometry::Point2D<double>> transformPoints;
			std::vector<Geometry::Point2D<double>> nonAffineTransformPoints;
			for (size_t i = 0; i < 8; i += 2)
			{
				transformPoints.push_back(Geometry::Point2D<double>(transformItems[i], transformItems[i + 1]));
				nonAffineTransformPoints.push_back(Geometry::Point2D<double>(nonAffineTransformItems[i], nonAffineTransformItems[i + 1]));
			}
			Geometry::Mesh<double> transformMesh = Geometry::Mesh<double>(transformPoints, 2, 2);
			nonAffineTransformMesh = Geometry::Mesh<double>(nonAffineTransformPoints, 2, 2);

			// Move the non affine transform mesh to the center after which we scale it by 
			// 1 / size to make sure our non affine mesh is in the scale of 0-1
			nonAffineTransformMesh.move(-transformMesh.bbox().minimum);
			auto size = Geometry::Point2D<double>{ 1.0f, 1.0f } / transformMesh.bbox().size();
			nonAffineTransformMesh.scale(size, { 0.0f, 0.0f });
		}
		// Convert back to array, note that we swap the point order here as Photoshop stores these in the order 
		// top-left, top-right, bottom-right, bottom-left
		auto points = nonAffineTransformMesh.points();
		return { points[0], points[1], points[3], points[2] };
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Geometry::Mesh<double> Warp::mesh() const
	{
		auto pointVec = std::vector<Geometry::Point2D<double>>(m_WarpPoints.begin(), m_WarpPoints.end());
		Geometry::Mesh<double> mesh(pointVec, m_NonAffineTransform, m_uDims, m_vDims);
		return mesh;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Geometry::BezierSurface Warp::surface() const
	{
		auto pointVec = std::vector<Geometry::Point2D<double>>(m_WarpPoints.begin(), m_WarpPoints.end());
		Geometry::BezierSurface surface(pointVec, m_uDims, m_vDims);
		return surface;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::warp_style(std::string style)
	{
		if (style != "warpCustom" && style != "warpNone")
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Invalid warp style received, expected 'warpCustom' or 'warpNone' but got '%s'", style.c_str());
		}
		m_WarpStyle = style;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::warp_value(double value)
	{
		m_WarpValue = value;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::warp_perspective(double value)
	{
		m_WarpPerspective = value;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::warp_perspective_other(double value)
	{
		m_WarpPerspectiveOther = value;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::warp_rotate(std::string rotate)
	{
		if (rotate != "Hrzn" && rotate != "Vrtc")
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Invalid warp rotate received, expected 'Hrzn' or 'Vrtc' but got '%s'", rotate.c_str());
		}
		m_WarpRotate = rotate;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::deserializeCommon(Warp& warpStruct, const Descriptors::Descriptor& warpDescriptor)
	{
		try
		{
			// 1. Retrieve warpStyle (Enumerated)
			const auto& warpStyle = warpDescriptor.at<Descriptors::Enumerated>("warpStyle");
			warpStruct.warp_style(warpStyle.m_Enum);

			// 2. Retrieve warpValue (double)
			const auto& warpValue = warpDescriptor.at<double>("warpValue");
			warpStruct.warp_value(warpValue);

			// 3. Retrieve warpPerspective (double)
			const auto& warpPerspective = warpDescriptor.at<double>("warpPerspective");
			const auto& warpPerspectiveOther = warpDescriptor.at<double>("warpPerspectiveOther");
			warpStruct.warp_perspective(warpPerspective);
			warpStruct.warp_perspective_other(warpPerspectiveOther);

			// 4. Retrieve warpRotate (Enumerated)
			const auto& warpRotate = warpDescriptor.at<Descriptors::Enumerated>("warpRotate");
			warpStruct.warp_rotate(warpRotate.m_Enum);

			// 5. Retrieve uOrder and vOrder (int32_t), always 4
			const auto& uOrder = warpDescriptor.at<int32_t>("uOrder");
			const auto& vOrder = warpDescriptor.at<int32_t>("vOrder");
			// We don't break here as files may work but redirect to the support page as I was not able to find any
			// files with non-4 values here

			if (uOrder != 4 || vOrder != 4)
			{
				PSAPI_LOG_WARNING("SmartObjectWarp",
					"U and V order were not 4 which is what was expected. Please submit a ticket on the github page with the file attachment"
				);
			}
		}
		catch (const std::runtime_error& e)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Internal Error: Invalid descriptor type encountered. Full exception: %s", e.what());
		}
		catch (const std::out_of_range& e)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Internal Error: Invalid descriptor key encountered. Full exception: %s", e.what());
		}
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::quilt_slices_x(std::vector<double> slices)
	{
		m_QuiltSlicesX = slices;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::quilt_slices_y(std::vector<double> slices)
	{
		m_QuiltSlicesY = slices;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::warp_type(WarpType type)
	{
		m_WarpType = type;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::non_affine_mesh(std::array<Geometry::Point2D<double>, 4> non_affine_transform_mesh)
	{
		m_NonAffineTransform = non_affine_transform_mesh;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	std::array<Geometry::Point2D<double>, 4> Warp::non_affine_mesh() const
	{
		return m_NonAffineTransform;
	}

}

PSAPI_NAMESPACE_END