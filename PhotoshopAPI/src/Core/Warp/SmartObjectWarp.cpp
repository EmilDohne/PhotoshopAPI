#include "SmartObjectWarp.h"

#include <OpenImageIO/imageio.h>

#include <memory>

PSAPI_NAMESPACE_BEGIN

namespace SmartObject
{

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Descriptors::Descriptor Warp::_serialize() const
	{
		if (m_WarpType == WarpType::quilt)
		{
			return _serialize(quilt_warp{});
		}
		return _serialize(normal_warp{});
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Descriptors::Descriptor Warp::_serialize(quilt_warp) const
	{
		// Helper lambda to reduce the signature a bit
		auto _os_key = [](Descriptors::Impl::OSTypes os_type)
			{
				return Descriptors::Impl::descriptorKeys.at(os_type);
			};


		Descriptors::Descriptor warp_descriptor("quiltWarp");
		_serialize_common(warp_descriptor);

		warp_descriptor.insert("deformNumRows", static_cast<int32_t>(m_vDims));
		warp_descriptor.insert("deformNumCols", static_cast<int32_t>(m_uDims));

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
	Descriptors::Descriptor Warp::_serialize(normal_warp) const
	{
		// Helper lambda to reduce the signature a bit
		auto _os_key = [](Descriptors::Impl::OSTypes os_type)
			{
				return Descriptors::Impl::descriptorKeys.at(os_type);
			};


		Descriptors::Descriptor warp_descriptor("warp");
		_serialize_common(warp_descriptor);
		
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
	std::tuple<Descriptors::List, Descriptors::List> Warp::_generate_transform_descriptors() const
	{
		std::tuple<Descriptors::List, Descriptors::List> out;

		// The transform is just the regular bbox as 4 corners in clockwise order starting
		// at the top-left
		Geometry::Point2D<double> top_left = m_Transform[0];
		Geometry::Point2D<double> top_rght = m_Transform[1];
		Geometry::Point2D<double> bot_rght = m_Transform[3];
		Geometry::Point2D<double> bot_left = m_Transform[2];

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

		Geometry::Point2D<double> non_aff_top_left = m_NonAffineTransform[0];
		Geometry::Point2D<double> non_aff_top_rght = m_NonAffineTransform[1];
		Geometry::Point2D<double> non_aff_bot_rght = m_NonAffineTransform[3];
		Geometry::Point2D<double> non_aff_bot_left = m_NonAffineTransform[2];

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
	void Warp::_serialize_common(Descriptors::Descriptor& warp_descriptor) const
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
	Descriptors::Descriptor Warp::_serialize_default(size_t width, size_t height)
	{
		// Set up the base warp object which has all the info we need
		Warp warp;
		warp._warp_style("warpNone");
		Geometry::BoundingBox<double> bbox;
		bbox.minimum = { 0.0f, 0.0f };
		bbox.maximum = { static_cast<double>(width), static_cast<double>(height) };
		warp._warp_bounds(bbox);

		Descriptors::Descriptor warp_descriptor("warp");
		warp._serialize_common(warp_descriptor);

		// The "customEnvelopeWarp" descriptor doesn't get set by photoshop

		return warp_descriptor;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Warp Warp::_deserialize(const Descriptors::Descriptor& warp_descriptor, const Descriptors::List& transform, const Descriptors::List& non_affine_transform, normal_warp)
	{
		Warp warp;
		warp._warp_type(WarpType::normal);
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
		Warp::_deserialize_common(warp, warp_descriptor);
		warp.affine_transform(Warp::_generate_affine_transform(transform));
		warp.non_affine_transform(Warp::_generate_non_affine_transform(non_affine_transform));

		return warp;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Warp Warp::_deserialize(const Descriptors::Descriptor& quilt_warp_descriptor, const Descriptors::List& transform, const Descriptors::List& non_affine_transform, quilt_warp)
	{
		Warp warp;
		warp._warp_type(WarpType::quilt);

		try
		{
			// Retrieve bounds descriptor (nested Descriptor)
			const auto& boundsDescriptor = quilt_warp_descriptor.at<Descriptors::Descriptor>("bounds");
			const auto& top = boundsDescriptor.at<double>("Top ");
			const auto& left = boundsDescriptor.at<double>("Left");
			const auto& bottom = boundsDescriptor.at<double>("Btom");
			const auto& right = boundsDescriptor.at<double>("Rght");

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
			warp._warp_bounds(Geometry::BoundingBox<double>({ left, top }, { right, bottom }));

			// Retrieve the quilt slices over x and y, these are probably the actual slice locations?
			// They are for some reason nested two levels deep
			const auto& quiltSlicesX = customEnvelopeWarp.at<Descriptors::ObjectArray>("quiltSliceX").at<Descriptors::UnitFloats>("quiltSliceX");
			const auto& quiltSlicesY = customEnvelopeWarp.at<Descriptors::ObjectArray>("quiltSliceY").at<Descriptors::UnitFloats>("quiltSliceY");

			warp._quilt_slices_x(quiltSlicesX.m_Values);
			warp._quilt_slices_y(quiltSlicesY.m_Values);
		}
		catch (const std::runtime_error& e)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Internal Error: Invalid descriptor type encountered. Full exception: %s", e.what());
		}
		catch (const std::out_of_range& e)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Internal Error: Invalid descriptor key encountered. Full exception: %s", e.what());
		}

		Warp::_deserialize_common(warp, quilt_warp_descriptor);
		warp.affine_transform(Warp::_generate_affine_transform(transform));
		warp.non_affine_transform(Warp::_generate_non_affine_transform(non_affine_transform));

		return warp;
	}



	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	std::array<Geometry::Point2D<double>, 4> Warp::_generate_affine_transform(const Descriptors::List& transform)
	{
		std::vector<double> transform_items = transform.as<double>();
		if (transform_items.size() != 8)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Invalid transform encountered, expected it to be of size 8, instead got %zu", transform_items.size());
		}

		std::vector<Geometry::Point2D<double>> points;
		for (size_t i = 0; i < 8; i += 2)
		{
			points.push_back(Geometry::Point2D<double>(transform_items[i], transform_items[i + 1]));
		}

		// Convert back to array, note that we swap the point order here as Photoshop stores these in the order 
		// top-left, top-right, bottom-right, bottom-left
		return { points[0], points[1], points[3], points[2] };
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	std::array<Geometry::Point2D<double>, 4> Warp::_generate_non_affine_transform(const Descriptors::List& non_affine_transform)
	{
		std::vector<double> nonAffineTransformItems = non_affine_transform.as<double>();
		if (nonAffineTransformItems.size() != 8)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Invalid transform and non-affine transform encountered, expected both to be of size 8, instead got %zu", nonAffineTransformItems.size());
		}


		std::vector<Geometry::Point2D<double>> nonAffineTransformPoints;
		{
			for (size_t i = 0; i < 8; i += 2)
			{
				nonAffineTransformPoints.push_back(Geometry::Point2D<double>(nonAffineTransformItems[i], nonAffineTransformItems[i + 1]));
			}
		}
		// Convert back to array, note that we swap the point order here as Photoshop stores these in the order 
		// top-left, top-right, bottom-right, bottom-left
		return { nonAffineTransformPoints[0], nonAffineTransformPoints[1], nonAffineTransformPoints[3], nonAffineTransformPoints[2] };
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Geometry::QuadMesh<double> Warp::mesh() const
	{
		auto point_vec = get_transformed_source_points();
		Geometry::QuadMesh<double> mesh(point_vec, m_uDims, m_vDims);

		return mesh;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Geometry::BezierSurface Warp::surface() const
	{
		auto point_vec = get_transformed_source_points();

		// Pass the quilt slices to apply the warp biasing
		if (m_WarpType == WarpType::quilt)
		{
			Geometry::BezierSurface surface(point_vec, m_uDims, m_vDims, m_QuiltSlicesX, m_QuiltSlicesY);
			return surface;
		}

		Geometry::BezierSurface surface(point_vec, m_uDims, m_vDims);
		return surface;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool Warp::no_op() const
	{
		// A no-op in this case is just where both transforms are identical
		return Geometry::Point2D<double>::equal(m_Transform[0], m_NonAffineTransform[0]) &&
			Geometry::Point2D<double>::equal(m_Transform[1], m_NonAffineTransform[1]) &&
			Geometry::Point2D<double>::equal(m_Transform[2], m_NonAffineTransform[2]) &&
			Geometry::Point2D<double>::equal(m_Transform[3], m_NonAffineTransform[3]);
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
			auto warp_mesh = warp_surface.mesh(25, 25, false);
			bbox = warp_mesh.bbox();
		}
		else
		{
			Geometry::QuadMesh<double> mesh(get_transformed_source_points(), m_uDims, m_vDims);
			bbox = mesh.bbox();
		}

		return bbox;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::_warp_style(std::string style)
	{
		if (style != "warpCustom" && style != "warpNone")
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Invalid warp style received, expected 'warpCustom' or 'warpNone' but got '%s'", style.c_str());
		}
		m_WarpStyle = style;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::_warp_value(double value)
	{
		m_WarpValue = value;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::_warp_perspective(double value)
	{
		m_WarpPerspective = value;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::_warp_perspective_other(double value)
	{
		m_WarpPerspectiveOther = value;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::_warp_rotate(std::string rotate)
	{
		if (rotate != "Hrzn" && rotate != "Vrtc")
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Invalid warp rotate received, expected 'Hrzn' or 'Vrtc' but got '%s'", rotate.c_str());
		}
		m_WarpRotate = rotate;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::_deserialize_common(Warp& warpStruct, const Descriptors::Descriptor& warpDescriptor)
	{
		try
		{
			// 1. Retrieve warpStyle (Enumerated)
			const auto& warpStyle = warpDescriptor.at<Descriptors::Enumerated>("warpStyle");
			warpStruct._warp_style(warpStyle.m_Enum);

			// 2. Retrieve warpValue (double)
			const auto& warpValue = warpDescriptor.at<double>("warpValue");
			warpStruct._warp_value(warpValue);

			// 3. Retrieve warpPerspective (double)
			const auto& warpPerspective = warpDescriptor.at<double>("warpPerspective");
			const auto& warpPerspectiveOther = warpDescriptor.at<double>("warpPerspectiveOther");
			warpStruct._warp_perspective(warpPerspective);
			warpStruct._warp_perspective_other(warpPerspectiveOther);

			const auto& warpBounds = warpDescriptor.at<Descriptors::Descriptor>("bounds");
			const auto top = warpBounds.at<double>("Top ");
			const auto left = warpBounds.at<double>("Left");
			const auto bottom = warpBounds.at<double>("Btom");
			const auto right = warpBounds.at<double>("Rght");
			Geometry::BoundingBox<double> bbox;
			bbox.minimum = { left, top };
			bbox.maximum = { right, bottom };
			warpStruct._warp_bounds(bbox);

			// 4. Retrieve warpRotate (Enumerated)
			const auto& warpRotate = warpDescriptor.at<Descriptors::Enumerated>("warpRotate");
			warpStruct._warp_rotate(warpRotate.m_Enum);

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
	void Warp::_quilt_slices_x(std::vector<double> slices)
	{
		m_QuiltSlicesX = slices;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::_quilt_slices_y(std::vector<double> slices)
	{
		m_QuiltSlicesY = slices;
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::_warp_type(WarpType type)
	{
		m_WarpType = type;
	}

	
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	SmartObject::Warp::WarpType Warp::_warp_type() const
	{
		return m_WarpType;
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

		std::array<Geometry::Point2D<double>, 4> transform;
		transform[0] = { static_cast<double>(0),		static_cast<double>(0)		};
		transform[1] = { static_cast<double>(width),	static_cast<double>(0)		};
		transform[2] = { static_cast<double>(0),		static_cast<double>(height) };
		transform[3] = { static_cast<double>(width),	static_cast<double>(height) };

		// Generate the points for warp in the coordinate space [0 - width] and [0 - height]
		std::vector<Geometry::Point2D<double>> points(u_dimensions * v_dimensions);
		for (size_t v = 0; v < v_dimensions; ++v)
		{
			double v_coord = (static_cast<double>(height) / (v_dimensions - 1)) * v;
			for (size_t u = 0; u < u_dimensions; ++u)
			{
				size_t idx = v * u_dimensions + u;
				double u_coord = (static_cast<double>(width) / (u_dimensions - 1)) * u;

				points[idx] = Geometry::Point2D<double>(u_coord, v_coord);
			}
		}

		// "Regular" warp as far as photoshop is concerned
		if (u_dimensions == 4 && v_dimensions == 4)
		{
			Warp warp(points, u_dimensions, v_dimensions);
			warp.affine_transform(transform[0], transform[1], transform[2], transform[3]);
			warp.non_affine_transform(transform[0], transform[1], transform[2], transform[3]);
			return warp;
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
		warp.affine_transform(transform[0], transform[1], transform[2], transform[3]);
		warp._quilt_slices_x(quilt_x_positions);
		warp._quilt_slices_y(quilt_y_positions);

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
	void Warp::_warp_bounds(Geometry::BoundingBox<double> bounds)
	{
		m_Bounds[0] = bounds.minimum.y;
		m_Bounds[1] = bounds.minimum.x;
		m_Bounds[2] = bounds.maximum.y;
		m_Bounds[3] = bounds.maximum.x;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Geometry::BoundingBox<double> Warp::_warp_bounds() const
	{
		Geometry::BoundingBox<double> bbox({ m_Bounds[1], m_Bounds[0] }, { m_Bounds[2], m_Bounds[3] });
		return bbox;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	Warp::Warp(std::vector<Geometry::Point2D<double>> warp, size_t u_dims, size_t v_dims)
	{
		Warp::validate_u_v_dims(u_dims, v_dims);

		if (u_dims == 4 && v_dims == 4)
		{
			m_WarpType = WarpType::normal;
		}
		else
		{
			m_WarpType = WarpType::quilt;
		}

		m_WarpPoints = warp;
		m_uDims = u_dims;
		m_vDims = v_dims;

		auto bbox = Geometry::BoundingBox<double>::compute(m_WarpPoints);
		m_Bounds[0] = bbox.minimum.y;
		m_Bounds[1] = bbox.minimum.x;
		m_Bounds[2] = bbox.maximum.y;
		m_Bounds[3] = bbox.maximum.x;

		m_Transform[0] = bbox.minimum;
		m_Transform[1] = { bbox.maximum.x, bbox.minimum.y };
		m_Transform[2] = { bbox.minimum.x, bbox.maximum.y };
		m_Transform[0] = bbox.maximum;

		m_NonAffineTransform = m_Transform;
	}

	
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bool Warp::operator==(const Warp& other) const
	{
		const auto other_pts = other.points();
		const auto other_affine = other.affine_transform();
		const auto other_non_affine = other.non_affine_transform();

		if (other_pts.size() != m_WarpPoints.size())
		{
			return false;
		}

		/// Check both the warp points and non affine points within the epsilon of Point2D::equal
		for (size_t i = 0; i < m_WarpPoints.size(); ++i)
		{
			auto equal = Geometry::Point2D<double>::equal(other_pts[i], m_WarpPoints[i]);
			if (!equal)
			{
				return false;
			}
		}

		for (size_t i = 0; i < m_Transform.size(); ++i)
		{
			auto equal = Geometry::Point2D<double>::equal(other_affine[i], m_Transform[i]);
			if (!equal)
			{
				return false;
			}
		}
		for (size_t i = 0; i < m_NonAffineTransform.size(); ++i)
		{
			auto equal = Geometry::Point2D<double>::equal(other_non_affine[i], m_NonAffineTransform[i]);
			if (!equal)
			{
				return false;
			}
		}
		return true;
	}

	
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::affine_transform(Geometry::Point2D<double> top_left, Geometry::Point2D<double> top_right, Geometry::Point2D<double> bot_left, Geometry::Point2D<double> bot_right)
	{
		constexpr double epsilon = 1e-3;

		auto calculate_slope = [](Geometry::Point2D<double> p1, Geometry::Point2D<double> p2) -> double 
		{
			return std::abs(p1.y - p2.y) > epsilon ? (p2.x - p1.x) / (p2.y - p1.y) : std::numeric_limits<double>::infinity();
		};

		auto slope_edge_top = calculate_slope(top_left, top_right);
		auto slope_edge_bot = calculate_slope(bot_left, bot_right);

		if (std::abs(slope_edge_bot - slope_edge_top) > epsilon)
		{
			PSAPI_LOG_ERROR("Warp",
				"Invalid affine transformation encountered, the line formed by top_left->top_right does not have the same slope"
				" as the line formed by bot_left->bot_right within epsilon %f. These lines must be perpendicular", epsilon);
		}

		auto slope_edge_left = calculate_slope(top_left, bot_left);
		auto slope_edge_right = calculate_slope(top_right, bot_right);

		if (std::abs(slope_edge_left - slope_edge_right) > epsilon)
		{
			PSAPI_LOG_ERROR("Warp",
				"Invalid affine transformation encountered, the line formed by top_left->bot_left does not have the same slope"
				" as the line formed by top_right->bot_right within epsilon %f. These lines must be perpendicular", epsilon);
		}

		m_Transform[0] = top_left;
		m_Transform[1] = top_right;
		m_Transform[2] = bot_left;
		m_Transform[3] = bot_right;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void Warp::non_affine_transform(Geometry::Point2D<double> top_left, Geometry::Point2D<double> top_right, Geometry::Point2D<double> bot_left, Geometry::Point2D<double> bot_right)
	{
		m_NonAffineTransform[0] = top_left;
		m_NonAffineTransform[1] = top_right;
		m_NonAffineTransform[2] = bot_left;
		m_NonAffineTransform[3] = bot_right;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	std::vector<Geometry::Point2D<double>> Warp::get_transformed_source_points() const
	{
		std::vector<Geometry::Point2D<double>> points = m_WarpPoints;

		// Create and apply the affine transform homography
		{
			auto bbox = Geometry::BoundingBox<double>(Geometry::Point2D<double>(m_Bounds[1], m_Bounds[0]), Geometry::Point2D<double>(m_Bounds[3], m_Bounds[2]));
			// auto bbox = Geometry::BoundingBox<double>::compute(points);
			auto homography_affine = Geometry::Operations::create_homography_matrix(bbox.as_quad(), m_Transform);

			Geometry::Operations::transform(points, homography_affine);
		}

		// Create and apply the non affine transform homography
		{
			auto homography_non_affine = Geometry::Operations::create_homography_matrix(m_Transform, m_NonAffineTransform);
			Geometry::Operations::transform(points, homography_non_affine);
		}
		
		return points;
	}

	void Warp::points(const std::vector<Geometry::Point2D<double>>& pts)
	{
		m_WarpPoints = pts;
	}

}

PSAPI_NAMESPACE_END