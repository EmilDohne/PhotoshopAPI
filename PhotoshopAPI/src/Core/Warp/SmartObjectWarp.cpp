#include "SmartObjectWarp.h"


#include <OpenImageIO/imageio.h>

#include <memory>

PSAPI_NAMESPACE_BEGIN


Descriptors::Descriptor SmartObjectWarp::serialize() const
{
	return {};
}

SmartObjectWarp SmartObjectWarp::deserialize(const Descriptors::Descriptor& warpDescriptor)
{
	SmartObjectWarp obj{};

	try
	{

#pragma warning( push )
#pragma warning( disable: 4189 )
		// 1. Retrieve warpStyle (Enumerated)
		const auto& warpStyle = warpDescriptor.at<Descriptors::Enumerated>("warpStyle");

		// 2. Retrieve warpValue (double)
		const auto& warpValue = warpDescriptor.at<double>("warpValue");

		// 3. Retrieve warpPerspective (double)
		const auto& warpPerspective = warpDescriptor.at<double>("warpPerspective");

		// 4. Retrieve warpPerspectiveOther (double)
		const auto& warpPerspectiveOther = warpDescriptor.at<double>("warpPerspectiveOther");

		// 5. Retrieve warpRotate (Enumerated)
		const auto& warpRotate = warpDescriptor.at<Descriptors::Enumerated>("warpRotate");

		// 6. Retrieve bounds descriptor (nested Descriptor)
		const auto& boundsDescriptor = warpDescriptor.at<Descriptors::Descriptor>("bounds");
		obj.m_Bounds[0] = boundsDescriptor.at<double>("Top ");
		obj.m_Bounds[1] = boundsDescriptor.at<double>("Left");
		obj.m_Bounds[2] = boundsDescriptor.at<double>("Btom");
		obj.m_Bounds[3] = boundsDescriptor.at<double>("Rght");

		// 7. Retrieve uOrder (int32_t)
		const auto& uOrder = warpDescriptor.at<int32_t>("uOrder");

		// 8. Retrieve vOrder (int32_t)
		const auto& vOrder = warpDescriptor.at<int32_t>("vOrder");

		// 9. Retrieve customEnvelopeWarp descriptor (nested Descriptor)
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
		
		size_t i = 0;
		for (auto& elem : obj.m_WarpPoints)
		{
			elem = Point<double>(hrznValues[i], vrtcValues[i]);
			++i;
		}

		auto pts = std::vector<Point<double>>(obj.m_WarpPoints.begin(), obj.m_WarpPoints.end());
		auto pts_subd = subdivideCatmullClark(pts, 4, 4);
		write_img_pts(pts_subd, static_cast<size_t>(obj.m_Bounds[3]), static_cast<size_t>(obj.m_Bounds[2]), "Warp.jpg");

		PSAPI_LOG_ERROR("A", "A");
#pragma warning( pop )
	}
	catch (const std::runtime_error& e)
	{
		PSAPI_LOG_ERROR("SmartObjectWarp", "Internal Error: Invalid descriptor type encountered. Full exception: %s", e.what());
	}
	catch (const std::out_of_range& e)
	{
		PSAPI_LOG_ERROR("SmartObjectWarp", "Internal Error: Invalid descriptor key encountered. Full exception: %s", e.what());
	}
	return {};
}

PSAPI_NAMESPACE_END