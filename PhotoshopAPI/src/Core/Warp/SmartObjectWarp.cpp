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
		if (m_WarpType == WarpType::quilt)
		{
			return serialize(quilt_warp{});
		}
		return serialize(normal_warp{});
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Descriptors::Descriptor Warp::serialize(quilt_warp) const
	{
		// Helper lambda to reduce the signature a bit
		auto _os_key = [](Descriptors::Impl::OSTypes os_type)
			{
				return Descriptors::Impl::descriptorKeys.at(os_type);
			};


		Descriptors::Descriptor warp_descriptor("quiltWarp");
		serialize_common(warp_descriptor);

		warp_descriptor.insert("deformNumRows", static_cast<int32_t>(m_uDims));
		warp_descriptor.insert("deformNumCols", static_cast<int32_t>(m_vDims));

		// This is where the actual warp information gets stored.
		Descriptors::Descriptor custom_envelope_warp("customEnvelopeWarp");

		// Store the quilt information
		{
			// X Slices
			Descriptors::ObjectArray quilt_slice_x("quiltSliceX", _os_key(Descriptors::Impl::OSTypes::ObjectArray));
			quilt_slice_x.m_ItemsCount = static_cast<uint32_t>(m_QuiltSlicesX.size());
			quilt_slice_x.m_ClassID = "UntF";

			Descriptors::UnitFloats slice_values_x(
				"quiltSliceX", 
				_os_key(Descriptors::Impl::OSTypes::UnitFloats), 
				Descriptors::Impl::UnitFloatType::Pixel,
				m_QuiltSlicesX
				);

			quilt_slice_x.insert("quiltSliceX", slice_values_x);

			// Y slices
			Descriptors::ObjectArray quilt_slice_y("quiltSliceY", _os_key(Descriptors::Impl::OSTypes::ObjectArray));
			quilt_slice_y.m_ItemsCount = static_cast<uint32_t>(m_QuiltSlicesY.size());
			quilt_slice_y.m_ClassID = "UntF";

			Descriptors::UnitFloats slice_values_y(
				"quiltSliceY",
				_os_key(Descriptors::Impl::OSTypes::UnitFloats),
				Descriptors::Impl::UnitFloatType::Pixel,
				m_QuiltSlicesY
			);

			quilt_slice_y.insert("quiltSliceY", slice_values_y);

			// Insert them into the warp, these go first before the envelope warp
			custom_envelope_warp.insert("quiltSliceX", quilt_slice_x);
			custom_envelope_warp.insert("quiltSliceY", quilt_slice_y);
		}


		// Store the mesh points
		{
			Descriptors::ObjectArray mesh_points("meshPoints", _os_key(Descriptors::Impl::OSTypes::ObjectArray));
			// This isn't a mistake, even though there's only 2 UnitFloats descriptors in here the m_ItemsCount
			// instead stores the number of items in the sub-descriptors
			mesh_points.m_ItemsCount = static_cast<int32_t>(m_WarpPoints.size());
			mesh_points.m_ClassID = "rationalPoint";

			Descriptors::UnitFloats horizontal_values("Hrzn", _os_key(Descriptors::Impl::OSTypes::UnitFloats));
			Descriptors::UnitFloats vertical_values("Vrtc", _os_key(Descriptors::Impl::OSTypes::UnitFloats));

			horizontal_values.m_UnitType = Descriptors::Impl::UnitFloatType::Pixel;
			vertical_values.m_UnitType = Descriptors::Impl::UnitFloatType::Pixel;

			for (const Geometry::Point2D<double> warp_point : m_WarpPoints)
			{
				horizontal_values.m_Values.push_back(warp_point.x);
				vertical_values.m_Values.push_back(warp_point.y);
			}

			mesh_points.insert("Hrzn", horizontal_values);
			mesh_points.insert("Vrtc", vertical_values);

			custom_envelope_warp.insert("meshPoints", mesh_points);
		}
		warp_descriptor.insert("customEnvelopeWarp", custom_envelope_warp);

		return warp_descriptor;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Descriptors::Descriptor Warp::serialize(normal_warp) const
	{
		// Helper lambda to reduce the signature a bit
		auto _os_key = [](Descriptors::Impl::OSTypes os_type)
			{
				return Descriptors::Impl::descriptorKeys.at(os_type);
			};


		Descriptors::Descriptor warp_descriptor("warp");
		serialize_common(warp_descriptor);
		
		// This is where the actual warp information gets stored.
		Descriptors::Descriptor custom_envelope_warp("customEnvelopeWarp");
		{
			Descriptors::ObjectArray mesh_points("meshPoints", _os_key(Descriptors::Impl::OSTypes::ObjectArray));
			// This isn't a mistake, even though there's only 2 UnitFloats descriptors in here the m_ItemsCount
			// instead stores the number of items in the sub-descriptors
			mesh_points.m_ItemsCount = static_cast<int32_t>(m_WarpPoints.size());
			mesh_points.m_ClassID = "rationalPoint";

			Descriptors::UnitFloats horizontal_values("Hrzn", _os_key(Descriptors::Impl::OSTypes::UnitFloats));
			Descriptors::UnitFloats vertical_values("Vrtc", _os_key(Descriptors::Impl::OSTypes::UnitFloats));

			horizontal_values.m_UnitType = Descriptors::Impl::UnitFloatType::Pixel;
			vertical_values.m_UnitType = Descriptors::Impl::UnitFloatType::Pixel;

			for (const Geometry::Point2D<double> warp_point : m_WarpPoints)
			{
				horizontal_values.m_Values.push_back(warp_point.x);
				vertical_values.m_Values.push_back(warp_point.y);
			}

			mesh_points.insert("Hrzn", horizontal_values);
			mesh_points.insert("Vrtc", vertical_values);

			custom_envelope_warp.insert("meshPoints", mesh_points);
		}
		warp_descriptor.insert("customEnvelopeWarp", custom_envelope_warp);

		return warp_descriptor;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	std::tuple<Descriptors::List, Descriptors::List> Warp::generate_transform_descriptors(std::array<Geometry::Point2D<double>, 4> transform_) const
	{
		std::tuple<Descriptors::List, Descriptors::List> out;

		// The transform is just the regular bbox as 4 corners in clockwise order starting
		// at the top-left
		Geometry::Point2D<double> top_left = transform_[0];
		Geometry::Point2D<double> top_rght = transform_[1];
		Geometry::Point2D<double> bot_rght = transform_[2];
		Geometry::Point2D<double> bot_left = transform_[3];

		Geometry::Point2D<double> center = (top_left + bot_rght) / 2;

		{
			Descriptors::List transform("Trnf", Descriptors::Impl::descriptorKeys.at(Descriptors::Impl::OSTypes::List));

			transform.m_Items.push_back(top_left.x);
			transform.m_Items.push_back(top_left.y);

			transform.m_Items.push_back(top_rght.x);
			transform.m_Items.push_back(top_rght.y);

			transform.m_Items.push_back(bot_rght.x);
			transform.m_Items.push_back(bot_rght.y);

			transform.m_Items.push_back(bot_left.x);
			transform.m_Items.push_back(bot_left.y);

			std::get<0>(out) = transform;
		}
		// Here we can use the property that we scaled our non affine transform to be in the range [0 - 1] (with offsets)
		// to offset both to 0, 0 and then scale one by the other, after which we rescale
		auto top_left_center = top_left - center;
		auto top_rght_center = top_rght - center;
		auto bot_rght_center = bot_rght - center;
		auto bot_left_center = bot_left - center;

		// Go from [0 - 1] -> [-1 - 1]
		auto non_aff_top_left_center = (m_NonAffineTransform[0] - Geometry::Point2D<double>(0.5f, 0.5f)) * 2;
		auto non_aff_top_rght_center = (m_NonAffineTransform[1] - Geometry::Point2D<double>(0.5f, 0.5f)) * 2;
		auto non_aff_bot_rght_center = (m_NonAffineTransform[3] - Geometry::Point2D<double>(0.5f, 0.5f)) * 2;
		auto non_aff_bot_left_center = (m_NonAffineTransform[2] - Geometry::Point2D<double>(0.5f, 0.5f)) * 2;

		// Scale the centered coordinates
		auto extents = Geometry::Point2D<double>{ std::abs(top_left_center.x), std::abs(top_left_center.y) };
		auto non_aff_top_left = non_aff_top_left_center * extents;
		auto non_aff_top_rght = non_aff_top_rght_center * extents;
		auto non_aff_bot_rght = non_aff_bot_rght_center * extents;
		auto non_aff_bot_left = non_aff_bot_left_center * extents;

		// Transform them back into the center
		non_aff_top_left = non_aff_top_left + center;
		non_aff_top_rght = non_aff_top_rght + center;
		non_aff_bot_rght = non_aff_bot_rght + center;
		non_aff_bot_left = non_aff_bot_left + center;

		{
			Descriptors::List non_affine_transform("nonAffineTransform", Descriptors::Impl::descriptorKeys.at(Descriptors::Impl::OSTypes::List));
			
			non_affine_transform.m_Items.push_back(non_aff_top_left.x);
			non_affine_transform.m_Items.push_back(non_aff_top_left.y);

			non_affine_transform.m_Items.push_back(non_aff_top_rght.x);
			non_affine_transform.m_Items.push_back(non_aff_top_rght.y);

			non_affine_transform.m_Items.push_back(non_aff_bot_rght.x);
			non_affine_transform.m_Items.push_back(non_aff_bot_rght.y);

			non_affine_transform.m_Items.push_back(non_aff_bot_left.x);
			non_affine_transform.m_Items.push_back(non_aff_bot_left.y);

			std::get<1>(out) = non_affine_transform;
		}
		return out;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::serialize_common(Descriptors::Descriptor& warp_descriptor) const
	{
		// Helper lambda to reduce the signature a bit
		auto _os_key = [](Descriptors::Impl::OSTypes os_type)
			{
				return Descriptors::Impl::descriptorKeys.at(os_type);
			};

		auto warp_style = Descriptors::Enumerated("warpStyle", _os_key(Descriptors::Impl::OSTypes::Enumerated), "warpStyle", m_WarpStyle);
		warp_descriptor.insert("warpStyle", warp_style);

		warp_descriptor.insert("warpValue", m_WarpValue);
		warp_descriptor.insert("warpPerspective", m_WarpPerspective);
		warp_descriptor.insert("warpPerspectiveOther", m_WarpPerspectiveOther);

		auto warp_rotation = Descriptors::Enumerated("warpRotate", _os_key(Descriptors::Impl::OSTypes::Enumerated), "Ornt", m_WarpRotate);
		warp_descriptor.insert("warpRotate", warp_rotation);

		Descriptors::Descriptor bounds("classFloatRect");
		{
			auto top = m_Bounds[0];
			auto left = m_Bounds[1];
			auto bottom = m_Bounds[2];
			auto right = m_Bounds[3];

			bounds.insert("Top ", top);
			bounds.insert("Left", left);
			bounds.insert("Btom", bottom);
			bounds.insert("Rght", right);
		}
		warp_descriptor.insert("bounds", bounds);

		warp_descriptor.insert("uOrder", m_uOrder);
		warp_descriptor.insert("vOrder", m_vOrder);

	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Descriptors::Descriptor Warp::serialize_default(size_t width, size_t height)
	{
		// Set up the base warp object which has all the info we need
		Warp warp;
		warp.warp_style("warpNone");
		Geometry::BoundingBox<double> bbox;
		bbox.minimum = { 0.0f, 0.0f };
		bbox.maximum = { static_cast<double>(width), static_cast<double>(height)};
		warp.warp_bounds(bbox);

		Descriptors::Descriptor warp_descriptor("warp");
		warp.serialize_common(warp_descriptor);

		// The "customEnvelopeWarp" descriptor doesn't get set by photoshop

		return warp_descriptor;
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
			warp.quilt_slices_y(quiltSlicesY.m_Values);
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
	Geometry::Point2D<double> Warp::point(size_t u_idx, size_t v_idx) const
	{
		if (u_idx > m_uDims - 1)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Invalid u index %zu provide, maximum u dimensions are %zu", u_idx, m_uDims - 1);
		}
		if (v_idx > m_vDims - 1)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Invalid v index %zu provide, maximum v dimensions are %zu", v_idx, m_vDims - 1);
		}
		size_t subindex = v_idx * m_uDims + u_idx;
		if (subindex > m_WarpPoints.size() - 1)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp",
				"Internal Error: The calculated subindex %zu would exceed the points' maximum indexable value of %zu",
				subindex, m_WarpPoints.size() - 1
			);
		}

		return m_WarpPoints[subindex];
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Geometry::Point2D<double>& Warp::point(size_t u_idx, size_t v_idx)
	{
		if (u_idx > m_uDims - 1)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Invalid u index %zu provide, maximum u dimensions are %zu", u_idx, m_uDims - 1);
		}
		if (v_idx > m_vDims - 1)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Invalid v index %zu provide, maximum v dimensions are %zu", v_idx, m_vDims - 1);
		}
		size_t subindex = v_idx * m_uDims + u_idx;
		if (subindex > m_WarpPoints.size() - 1)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", 
				"Internal Error: The calculated subindex %zu would exceed the points' maximum indexable value of %zu",
				subindex, m_WarpPoints.size() - 1
			);
		}

		return m_WarpPoints[subindex];
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Geometry::BoundingBox<double> Warp::bounds(bool consider_bezier /*= true*/) const
	{
		Geometry::BoundingBox<double> bbox;

		if (consider_bezier)
		{
			auto warp_surface = surface();
			auto warp_mesh = warp_surface.mesh(25, 25, m_NonAffineTransform);
			bbox = warp_mesh.bbox();
		}
		else
		{
			Geometry::Mesh<double> mesh(m_WarpPoints, m_NonAffineTransform, m_uDims, m_vDims);
			bbox = mesh.bbox();
		}

		return bbox;
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

			const auto& warpBounds = warpDescriptor.at<Descriptors::Descriptor>("bounds");
			const auto top = warpBounds.at<double>("Top ");
			const auto left = warpBounds.at<double>("Left");
			const auto bottom = warpBounds.at<double>("Btom");
			const auto right = warpBounds.at<double>("Rght");
			Geometry::BoundingBox<double> bbox;
			bbox.minimum = { left, top };
			bbox.maximum = { right, bottom };
			warpStruct.warp_bounds(bbox);

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
	SmartObject::Warp::WarpType Warp::warp_type() const
	{
		return m_WarpType;
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


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	SmartObject::Warp Warp::generate_default(size_t width, size_t height)
	{
		return Warp::generate_default(width, height, 4, 4);
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	SmartObject::Warp Warp::generate_default(size_t width, size_t height, size_t u_dimensions, size_t v_dimensions)
	{
		Warp::validate_u_v_dims(u_dimensions, v_dimensions);
		size_t u_patches = 1 + (u_dimensions - 4) / 3;
		size_t v_patches = 1 + (v_dimensions - 4) / 3;

		// Generate the points for warp in the coordinate space [0 - width] and [0 - height]
		std::vector<Geometry::Point2D<double>> points(u_dimensions * v_dimensions);
		for (size_t v = 0; v < v_dimensions; ++v)
		{
			double v_coord = (static_cast<double>(1.0f) / v_dimensions) * v;
			for (size_t u = 0; u < u_dimensions; ++u)
			{
				size_t idx = v * u_dimensions + u;
				double u_coord = (static_cast<double>(1.0f) / u_dimensions) * u;

				points[idx] = Geometry::Point2D<double>(u_coord, v_coord);
			}
		}

		// "Regular" warp as far as photoshop is concerned
		if (u_dimensions == 4 && v_dimensions == 4)
		{
			return Warp(std::move(points), u_dimensions, v_dimensions);
		}

		// If we have more than 1 patch in even one dimensions this is a "quilt" warp
		// and the quilt slice positions along the x and y in our case are just evenly spaced from [0 - width]
		// and [0 - height] with num_patches + 2 coordinates.
		// If we have a e.g. a 2x2 patch (of cubic bezier curves) for a width of 4000 it will look like this:
		//
		// [-0.6, 2000.0, 4000.6]
		// 
		// The .6 offset is likely because in view they are interpreted as integers and adobe wanted to avoid
		// these rounding to 0 and drawing the line double or something like that.
		std::vector<double> quilt_x_positions(u_patches + 2);
		std::vector<double> quilt_y_positions(v_patches + 2);
		
		auto generate_default_quilt = [](std::vector<double>& quilt, size_t size, size_t num_patches)
			{
				assert(quilt.size() >= 3);

				quilt[0] = static_cast<double>(-0.6f);
				quilt[quilt.size() - 1] = static_cast<double>(size) + .6f;

				for (size_t i = 0; i < num_patches; ++i)
				{
					double increment = static_cast<double>(size) / (quilt.size() - 1);
					size_t idx = i + 1;
					quilt[idx] = increment * idx;
				}
			};

		generate_default_quilt(quilt_x_positions, width, u_patches);
		generate_default_quilt(quilt_y_positions, height, v_patches);

		Warp warp(points, u_dimensions, v_dimensions);
		warp.quilt_slices_x(quilt_x_positions);
		warp.quilt_slices_y(quilt_y_positions);

		return warp;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::validate_u_v_dims(size_t u_dimensions, size_t v_dimensions)
	{
		if (u_dimensions < 4)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Warp requires at least 4 u-dimensions, got %zu", u_dimensions);
		}
		if (v_dimensions < 4)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Warp requires at least 4 v-dimensions, got %zu", v_dimensions);
		}
		if ((u_dimensions - 4) % 3 != 0)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp",
				"Warp requires 4 + n * 3 u-dimensions to generate cubic bezier patches e.g. 4, 7, 10 etc. Instead got %zu",
				u_dimensions);
		}
		if ((v_dimensions - 4) % 3 != 0)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp",
				"Warp requires 4 + n * 3 v-dimensions to generate cubic bezier patches e.g. 4, 7, 10 etc. Instead got %zu",
				v_dimensions);
		}
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::warp_bounds(Geometry::BoundingBox<double> bounds)
	{
		m_Bounds[0] = bounds.minimum.y;
		m_Bounds[1] = bounds.minimum.x;
		m_Bounds[2] = bounds.maximum.y;
		m_Bounds[3] = bounds.maximum.x;
	}
	

}

PSAPI_NAMESPACE_END