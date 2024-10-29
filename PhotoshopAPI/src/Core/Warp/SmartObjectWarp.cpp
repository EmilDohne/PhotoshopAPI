#include "SmartObjectWarp.h"


#include <OpenImageIO/imageio.h>

#include <memory>

PSAPI_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
Descriptors::Descriptor SmartObjectWarp::serialize() const
{
	throw std::runtime_error("Unimplemented");
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
SmartObjectWarp SmartObjectWarp::deserialize(const Descriptors::Descriptor& warpDescriptor, SmartObjectWarp::WarpType type)
{
	if (type == WarpType::normal)
	{
		return SmartObjectWarp::deserializeNormal(warpDescriptor);
	}
	else
	{
		return SmartObjectWarp::deserializeQuilt(warpDescriptor);
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
Geometry::Mesh<double> SmartObjectWarp::mesh() const
{
	auto pointVec = std::vector<Geometry::Point2D<double>>(m_WarpPoints.begin(), m_WarpPoints.end());
	Geometry::Mesh<double> mesh(pointVec, m_uDims, m_vDims);
	return mesh;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
Geometry::BezierSurface SmartObjectWarp::surface() const
{
	auto pointVec = std::vector<Geometry::Point2D<double>>(m_WarpPoints.begin(), m_WarpPoints.end());
	Geometry::BezierSurface surface(pointVec, m_uDims, m_vDims);
	return surface;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
SmartObjectWarp SmartObjectWarp::deserializeQuilt(const Descriptors::Descriptor& warpDescriptor)
{
	SmartObjectWarp warp;
	try
	{
		// Retrieve bounds descriptor (nested Descriptor)
		const auto& boundsDescriptor = warpDescriptor.at<Descriptors::Descriptor>("bounds");
		const auto& top = boundsDescriptor.at<double>("Top ");
		const auto& left = boundsDescriptor.at<double>("Left");
		const auto& bottom = boundsDescriptor.at<double>("Btom");
		const auto& right = boundsDescriptor.at<double>("Rght");

		// Retrieve deformNumRows and deformNumCols (int32_t)
		const auto& deformNumRows = warpDescriptor.at<int32_t>("deformNumRows");
		const auto& deformNumCols = warpDescriptor.at<int32_t>("deformNumCols");

		// Retrieve customEnvelopeWarp descriptor (nested Descriptor)
		const auto& customEnvelopeWarp = warpDescriptor.at<Descriptors::Descriptor>("customEnvelopeWarp");
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
		warp = SmartObjectWarp(warp_points, deformNumCols, deformNumRows, { top, left, bottom, right });


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

	SmartObjectWarp::deserializeCommon(warp, warpDescriptor);

	return warp;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
SmartObjectWarp SmartObjectWarp::deserializeNormal(const Descriptors::Descriptor& warpDescriptor)
{
	SmartObjectWarp obj;
	try
	{
		// Retrieve bounds descriptor (nested Descriptor)
		const auto& boundsDescriptor = warpDescriptor.at<Descriptors::Descriptor>("bounds");
		obj.m_Bounds[0] = boundsDescriptor.at<double>("Top ");
		obj.m_Bounds[1] = boundsDescriptor.at<double>("Left");
		obj.m_Bounds[2] = boundsDescriptor.at<double>("Btom");
		obj.m_Bounds[3] = boundsDescriptor.at<double>("Rght");

		// Retrieve customEnvelopeWarp descriptor (nested Descriptor)
		const auto& customEnvelopeWarp = warpDescriptor.at<Descriptors::Descriptor>("customEnvelopeWarp");
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

		obj.m_WarpPoints.reserve(hrznValues.size());
		for (size_t i = 0; i < hrznValues.size(); ++i)
		{
			obj.m_WarpPoints.push_back(Geometry::Point2D<double>(hrznValues[i], vrtcValues[i]));
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
	SmartObjectWarp::deserializeCommon(obj, warpDescriptor);

	return obj;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void SmartObjectWarp::warp_style(std::string style)
{
	if (style != "warpCustom" && style != "warpNone")
	{
		PSAPI_LOG_ERROR("SmartObjectWarp", "Invalid warp style received, expected 'warpCustom' or 'warpNone' but got '%s'", style.c_str());
	}
	m_WarpStyle = style;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void SmartObjectWarp::warp_value(double value)
{
	m_WarpValue = value;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void SmartObjectWarp::warp_perspective(double value)
{
	m_WarpPerspective = value;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void SmartObjectWarp::warp_perspective_other(double value)
{
	m_WarpPerspectiveOther = value;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void SmartObjectWarp::warp_rotate(std::string rotate)
{
	if (rotate != "Hrzn" && rotate != "Vrtc")
	{
		PSAPI_LOG_ERROR("SmartObjectWarp", "Invalid warp rotate received, expected 'Hrzn' or 'Vrtc' but got '%s'", rotate.c_str());
	}
	m_WarpRotate = rotate;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void SmartObjectWarp::deserializeCommon(SmartObjectWarp& warpStruct, const Descriptors::Descriptor& warpDescriptor)
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
void SmartObjectWarp::quilt_slices_x(std::vector<double> slices)
{
	m_QuiltSlicesX = slices;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void SmartObjectWarp::quilt_slices_y(std::vector<double> slices)
{
	m_QuiltSlicesY = slices;
}

PSAPI_NAMESPACE_END