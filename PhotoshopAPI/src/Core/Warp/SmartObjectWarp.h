#pragma once

#include "Macros.h"

#include "Core/Struct/DescriptorStructure.h"

#include "Core/Render/Render.h"
#include "Core/Render/ImageBuffer.h"
#include "Core/Geometry/Point.h"
#include "Core/Geometry/Mesh.h"
#include "Core/Geometry/MeshOperations.h"
#include "Core/Geometry/BezierSurface.h"


#include <array>


PSAPI_NAMESPACE_BEGIN

namespace SmartObject
{

	/// Base warp structure, these encompass the warps found under Edit -> Transform which are:
	/// 
	/// Edit/Transform/Skew
	/// 
	/// Edit/Transform/Distort
	/// 
	/// Edit/Transform/Perspective
	/// 
	/// Edit/Transform/Warp
	/// 
	/// These are stored as part of the descriptor found on the PlacedLayer or PlacedLayerData Tagged Blocks.
	/// The data stored on the file does not differentiate between these types and they are all stored
	/// identically (the internal structure might change though)
	struct Warp
	{
		/// Tags for dispatch
		struct normal_warp {};
		/// Tags for dispatch
		struct quilt_warp {};

		enum class WarpType
		{
			normal,	/// Normal warps in this case define a 4x4 warp grid
			quilt,	/// Quilt warps are for arbitrary warp resolutions
		};

		Warp() = default;

		/// Initialize the warp struct from a set of geometric points describing a bezier surface with
		/// one or more quadratic bezier patches. These points are in scanline order (i.e. going first along the horizontal
		/// axis, then across the vertical axis). 
		/// 
		/// Being a set of quadratic bezier patches the dimensions across the u and v (x and y) must be `4` or `4 + n * 3`
		/// where `n` is the number of subdivisions and is greater than one. In simple terms, this means a valid number of 
		/// points per axis is 4, 7, 10, 13 etc.
		/// 
		/// \param warp The warp points in scanline order.
		/// \param u_dims The dimensions across the u (x)
		/// \param v_dims The dimensions across the v (y)
		Warp(std::vector<Geometry::Point2D<double>> warp, size_t u_dims, size_t v_dims);

		/// Check if the warp struct is valid, for now returns whether the warp points hold any data
		bool valid() { return m_WarpPoints.size() > 0; }

		/// Generate a Mesh from our warp structure, this is primarily used for directly
		/// visualizing the points. To get a subdivided Bezier representation use `surface().mesh()`
		Geometry::QuadMesh<double> mesh() const;

		/// Generate a bezier surface from this warp structure. This can be used e.g. for rendering
		Geometry::BezierSurface surface() const;

		/// Check if the warp resolves to a no-op. This means that all points for a given row/column
		/// lie on a single line and the non-affine transform is also a no op. If this is the case applying
		/// a warp can be skipped
		bool no_op() const;

		/// Apply the warp by warping the `image` into the `buffer` using the locally stored warp description. 
		/// The `buffer` passed should match the general resolution of the warp points. So if e.g. the warp points 
		/// are from { 0 - 4000, 0 - 2000 } the `buffer` parameter should cover these.
		/// If this isn't the case the function won't fail but the image will not contain the full warped picture.
		/// 
		/// \param buffer		The buffer to render into
		/// 
		/// \param image		The image do warp using the local warp struct
		/// 
		/// \param warp_mesh	A mesh to apply the warp with, mainly to be used as an optimization step if 
		///						you wish to apply the warp to multiple channels at the same time to only calculate the 
		///						mesh construction once. Can be gotten using `SmartObjectWarp::mesh()`
		/// 
		template <typename T>
		void apply(Render::ImageBuffer<T> buffer, Render::ConstImageBuffer<T> image, const Geometry::QuadMesh<double>& warp_mesh) const
		{
			PSAPI_PROFILE_FUNCTION();

			// Limit the computation of the warp to the region of interest (ROI) of the mesh itself.
			// that way we just skip any pixels that we know wont have any warp to it
			auto bbox = warp_mesh.bbox();
			size_t min_y = std::max<size_t>(static_cast<size_t>(std::round(bbox.minimum.y)), 0);
			size_t max_y = std::min<size_t>(static_cast<size_t>(std::round(bbox.maximum.y)), buffer.height - 1);
			size_t min_x = std::max<size_t>(static_cast<size_t>(std::round(bbox.minimum.x)), 0);
			size_t max_x = std::min<size_t>(static_cast<size_t>(std::round(bbox.maximum.x)), buffer.width - 1);

			std::vector<size_t> vertical_iter(max_y - min_y + 1);
			std::iota(vertical_iter.begin(), vertical_iter.end(), min_y);

			std::for_each(std::execution::par_unseq, vertical_iter.begin(), vertical_iter.end(), [&](size_t y)
				{
					constexpr double failure_condition = -1.0f;
					for (size_t x = min_x; x <= max_x; ++x)
					{
						// The way this works is that on creation of the mesh from the bezier we actually
						// initialize UV coordinates that are equally spaced, this is irrespective of the
						// divisions the bezier was created with. 
						// By then sampling the uv coordinate on the mesh for each pixel in the buffer 
						// (the position Point2D we initialize below) we essentially know what part
						// of the original image belongs to the warped mesh as we can treat the 
						// original (unwarped) image as a UV space from 0-1.
						// 
						// We then bilinearly sample the source image to avoid artifacts from nearest 
						// neighbour sampling. In the context of the SmartObjectWarp the resolution
						// of the source and target are identical with downscaling happening further
						// down the pipeline.

						size_t idx = y * buffer.width + x;
						auto position = Geometry::Point2D<double>(x, y);
						auto uv = warp_mesh.uv_coordinate(position);

						// If the uv coordinate is outside of the image we dont bother with it.
						// We can check against the exact -1.0f here as that is what we return
						if (uv == failure_condition)
						{
							continue;
						}

						buffer.buffer[idx] = image.sample_bilinear_uv(uv);
					}
				});
		}


		/// Apply the warp by warping the `image` into the `buffer` using the locally stored warp description. 
		/// The `buffer` passed should match the general resolution of the warp points. So if e.g. the warp points 
		/// are from { 0 - 4000, 0 - 2000 } the `buffer` parameter should cover these.
		/// If this isn't the case the function won't fail but the image will not contain the full warped picture.
		/// 
		/// \param buffer		The buffer to render into
		/// \param image		The image do warp using the local warp struct
		/// \param resolution	The resolution of warp geometry. Since we first convert the BezierSurface into a Mesh 
		///						this parameter dictates how many pixels apart each subdivision line should be. Do note
		///						that increasing this massively does not necessarily give a better result. Defaults to `25`.
		template <typename T>
		void apply(Render::ImageBuffer<T> buffer, Render::ConstImageBuffer<T> image, size_t resolution = 25) const
		{
			auto warp_surface = surface();
			auto warp_mesh = warp_surface.mesh(buffer.width / resolution, buffer.height / resolution);
			apply(buffer, image, warp_mesh, resolution);
		}

		/// Generates a default warp, this should be the main entry point if you wish to author a custom warp.
		/// 
		/// Internally this will author a 4x4 grid describing a cubic bezier patch whose points can be transformed
		/// by retrieving `point()` and applying the given transformation to it. These grid points are laid out as 
		/// follows:
		/// 
		/// 0  1  2  3
		/// 
		/// 4  5  6  7
		/// 
		/// 8  9  10 11
		/// 
		/// 12 13 14 15
		/// 
		/// Here point 0 would be the top left corner point. And points 1 and 4 the handles to the bezier.
		/// You now might wonder what the purpose of the center points are as they are not exposed in 
		/// Photoshop itself. 
		/// 
		/// These appear to be added to form a quad that is a parallelogram. We don't currently expose 
		/// any functionality for making this parallelogram from 3 points as the output is ambiguous.
		/// So if we continue with our previous example, to form the parallelogram we would have to
		/// modify point 5
		/// 
		/// If you wish to see how this 4x4 grid can look check out this page:
		/// https://github.com/EmilDohne/PhotoshopAPI/issues/90#issuecomment-2441823792
		/// 
		/// \param width The width of the warp, logically this should be the full image width.
		///				 In the context of a smart object for example this would be the width of the whole image,
		///				 not of the generated preview.
		/// \param height The height of the warp, logically this should be the full image height.
		///				 In the context of a smart object for example this would be the height of the whole image,
		///				 not of the generated preview.
		static Warp generate_default(size_t width, size_t height);

		/// Generates a default warp, this should be the main entry point if you wish to author a custom warp.
		/// 
		/// \note For most use cases you likely want to use the overload with just a width and height parameter
		///		  as that warp is easier to understand conceptually. Authoring warps using arbitray u and v dimensions
		///		  requires fairly good knowledge of how bezier surfaces composed of quadratic bezier patches look like.
		/// 
		/// Internally this will author a u*v grid describing a collection of cubic bezier patches whose points can be 
		/// transformed by retrieving `point()` and applying the given transformation to it. 
		/// These grid points are laid out as follows (for a 4x4 grid, for other dimensions this would change accordingly):
		/// 
		/// 0  1  2  3
		/// 
		/// 4  5  6  7
		/// 
		/// 8  9  10 11
		/// 
		/// 12 13 14 15
		/// 
		/// Here point 0 would be the top left corner point. And points 1 and 4 the handles to the bezier.
		/// You now might wonder what the purpose of the center points are as they are not exposed in 
		/// Photoshop itself. 
		/// 
		/// These appear to be added to form a quad that is a parallelogram. We don't currently expose 
		/// any functionality for making this parallelogram from 3 points as the output is ambiguous.
		/// So if we continue with our previous example, to form the parallelogram we would have to
		/// modify point 5
		/// 
		/// If you wish to see how a 4x4 grid can look check out this page:
		/// https://github.com/EmilDohne/PhotoshopAPI/issues/90#issuecomment-2441823792
		/// 
		/// \param width The width of the warp, logically this should be the full image width.
		///				 In the context of a smart object for example this would be the width of the whole image,
		///				 not of the generated preview.
		/// \param height The height of the warp, logically this should be the full image height.
		///				 In the context of a smart object for example this would be the height of the whole image,
		///				 not of the generated preview.
		/// \param u_dimensions The divisions in the u (x) dimension. These must follow the formula 4 + n * 3
		///						where n represents the number of horizontal bezier patches - 1. So if you wish
		///						to construct 3 bezier patches horizontally this would be 10.
		/// \param v_dimensions The divisions in the v (y) dimension. These must follow the formula 4 + n * 3
		///						where n represents the number of vertical bezier patches - 1. So if you wish
		///						to construct 3 bezier patches vertically this would be 10.
		static Warp generate_default(size_t width, size_t height, size_t u_dimensions, size_t v_dimensions);

		/// Get the warp point at the given u and v index. Checks for out of bounds index.
		Geometry::Point2D<double> point(size_t u_idx, size_t v_idx) const;

		/// Get a reference to the warp point at the given u and v index. Checks for out of bounds index.
		Geometry::Point2D<double>& point(size_t u_idx, size_t v_idx);

		/// Get the points this warp struct holds
		inline std::vector<Geometry::Point2D<double>> points() const { return m_WarpPoints; }

		/// Set the points this warp struct holds. Updates the bounding box.
		/// This assumes that the point ordering and resolution did not change.
		void points(const std::vector<Geometry::Point2D<double>>& pts);		

		/// Get the dimensions/subdivisions across the u (x) axis.
		inline size_t u_dimensions() const noexcept { return m_uDims; };

		/// Get the dimensions/subdivisions across the v (y) axis.
		inline size_t v_dimensions() const noexcept { return m_vDims; };

		/// Get the bounds of the smart object warp. These are recomputed on access
		/// so you are guaranteed to get the most up-to-date bounding box.
		/// 
		/// \param consider_bezier If this is set to true we first create a bezier surface
		///						   from the mesh giving us a precise bounding box while without this 
		///						   we may undershoot the bounding box. This carries some performance cost.
		///						   Defaults to true
		/// 
		/// \returns either a tight fitting bbox or an approximate bbox depending on whether
		///			 `consider_bezier` is set to true.
		Geometry::BoundingBox<double> bounds(bool consider_bezier = true) const;

		/// Retrieve the affine transform which describes 4 points in the order {top-left, top-right, bot-left, bot-right}
		/// These represent the corner coordinates of the quad that describes the transformation (move, rotate, scale and shear).
		/// 
		/// All the opposing lines of this quad are guaranteed to be parallel unlike the non-affine transform.
		/// 
		/// This represents the following step in the transformation pipeline:
		/// 
		/// Full image -> **Affine transform** -> Non-affine transform -> Warp
		inline std::array<Geometry::Point2D<double>, 4> affine_transform() const { return m_Transform; }

		// Set the affine transform for the SmartObjectWarp. The opposing sides of the quad formed by the four points
		/// must be parallel. It is recommended to use the transformation functions on the SmartObjectLayer such as
		/// `move()` `rotate()` `scale()` instead of modifying the quad directly for better and more intuitive control.
		inline void affine_transform(std::array<Geometry::Point2D<double>, 4> transform) { m_Transform = transform; }

		/// Set the affine transform for the SmartObjectWarp. The opposing sides of the quad formed by the four points
		/// must be parallel. It is recommended to use the transformation functions on the SmartObjectLayer such as
		/// `move()` `rotate()` `scale()` instead of modifying the quad directly for better and more intuitive control.
		void affine_transform (
			Geometry::Point2D<double> top_left,
			Geometry::Point2D<double> top_right,
			Geometry::Point2D<double> bot_left,
			Geometry::Point2D<double> bot_right
		);

		/// Retrieve the non-affine transform which describes 4 points in the order {top-left, top-right, bot-left, bot-right}.
		/// If no non-affine transform is present these are the same as affine_transform, pushing e.g. the top left and top right 
		/// outwards would represent a perspective transform.
		inline const std::array<Geometry::Point2D<double>, 4>& non_affine_transform() const { return m_NonAffineTransform; };

		/// Set the non-affine transform which describes 4 points in the order {top-left, top-right, bot-left, bot-right}.
		/// These are described as positions in world-space that for a no-op must be the same as affine_transform.
		/// When wanting to e.g. perspective warp the top corners you must push the first two points outwards or inwards.
		inline void non_affine_transform(std::array<Geometry::Point2D<double>, 4> transform) { m_NonAffineTransform = transform; };

		/// Set the non-affine transform which describes 4 points in the order {top-left, top-right, bot-left, bot-right}.
		/// These are described as positions in world-space that for a no-op must be the same as affine_transform.
		/// When wanting to e.g. perspective warp the top corners you must push the first two points outwards or inwards.
		void non_affine_transform(
			Geometry::Point2D<double> top_left,
			Geometry::Point2D<double> top_right,
			Geometry::Point2D<double> bot_left,
			Geometry::Point2D<double> bot_right
		);

		bool operator==(const Warp& other) const;

		// Internal API setters and getters operating on the low level structures. The users can use them but its highly
		// discouraged unless they have extensive knowledge of how they work and how they are to be modified.
		// ---------------------------------------------------------------------------------------------------------------------
		// ---------------------------------------------------------------------------------------------------------------------

		/// For internal API use:
		/// Create a "warp"/"quiltWarp" descriptor from this class ready to be stored on a PlacedLayer or PlacedLayerData tagged block
		Descriptors::Descriptor _serialize() const;

		/// For internal API use:
		/// Create the transform and non-affine transform descriptors
		std::tuple<Descriptors::List, Descriptors::List> _generate_transform_descriptors() const;

		/// For internal API use:
		/// Serialize the common parts between both quilt and normal warps
		void _serialize_common(Descriptors::Descriptor& warp_descriptor) const;

		/// For internal API use:
		/// 
		/// Create an empty "warp" descriptor from this class. This is to be used if the actual warp type
		/// is a quilt warp. Photoshop, if the warp is a "quiltWarp" stores a default initialized (but with the given
		/// width and height) "warp" descriptor to store after
		static Descriptors::Descriptor _serialize_default(size_t width, size_t height);

		/// For internal API use:
		/// Deserialize the SmartObjectWarp from a warp descriptor. In the context 
		/// of the smart object this would be at the "warp" key.
		static Warp _deserialize(const Descriptors::Descriptor& warp_descriptor, const Descriptors::List& transform, const Descriptors::List& non_affine_transform, normal_warp);
		
		/// For internal API use:
		/// Deserialize the SmartObjectWarp from a warp descriptor. In the context 
		/// of the smart object this would be at the "warp" key.
		static Warp _deserialize(const Descriptors::Descriptor& quilt_warp_descriptor, const Descriptors::List& transform, const Descriptors::List& non_affine_transform, quilt_warp);

		/// For internal API use:
		///
		/// Generates an affine transform mesh from the descriptor.
		static std::array<Geometry::Point2D<double>, 4> _generate_affine_transform(const Descriptors::List& transform);

		/// For internal API use:
		///
		/// Generates a non affine transform from the descriptor
		static std::array<Geometry::Point2D<double>, 4> _generate_non_affine_transform(const Descriptors::List& non_affine_transform);

		/// For internal API use:
		/// Set the warp style ("warpCustom" or "warpNone")
		void _warp_style(std::string style);

		/// For internal API use:
		/// Set the bounds using the bounding box
		void _warp_bounds(Geometry::BoundingBox<double> bounds);

		/// For internal API use:
		/// Get the bounds using the bounding box
		Geometry::BoundingBox<double> _warp_bounds() const;

		/// For internal API use:
		/// Set the warp value, unsure where and how this is used, usually 0.0f
		void _warp_value(double value);

		/// For internal API use:
		/// Set the warp perspective value, unsure where and how this is used, usually 0.0f
		void _warp_perspective(double value);

		/// For internal API use:
		/// Set the warp perspective other? value, unsure where and how this is used, usually 0.0f
		void _warp_perspective_other(double value);

		/// For internal API use:
		/// Set the warp rotate value, either "Hrzn" or "Vrtc"
		void _warp_rotate(std::string rotate);

		/// For internal API use:
		/// The positions of the quilt slices in terms of coordinates in the original image.
		/// if a slice is needed in the middle for example for an image with a width of 5000px
		/// the slice coordinate would be 2500 and the whole slices vect would be {0, 2500, 5000}
		void _quilt_slices_x(std::vector<double> slices);

		/// For internal API use:
		/// The positions of the quilt slices in terms of coordinates in the original image.
		/// if a slice is needed in the middle for example for an image with a height of 5000px
		/// the slice coordinate would be 2500 and the whole slices vect would be {0, 2500, 5000}
		void _quilt_slices_y(std::vector<double> slices);

		/// For internal API use:
		/// Set the type of warp. The two valid options are 'quilt' and 'normal' where normal is a 4x4 grid
		/// and quilt is anything beyond that (resolutions under 4x4 are not supported).
		void _warp_type(WarpType type);
		WarpType _warp_type() const;

	private:

		/// The warp points, this is a coordinate grid of u*v points going in scanline order from the top left 
		/// to the bottom right, here an example for a 4x4 grid
		/// 1  2  3  4
		/// 5  6  7  8
		/// 9  10 11 12
		/// 13 14 15 16
		/// 
		/// The points themselves don't describe coordinates on the canvas but instead on the resolution
		/// of the smart object. So if the canvas is 1000x500 pixels but the smart object's original resolution
		/// is 4000x2000 the points are the offsets from that. The warp points are represented as either a 
		/// singular cubic bezier patch in the context of a 
		std::vector<Geometry::Point2D<double>> m_WarpPoints;

		/// The bounds of the geometry. Defined as Top, Left, Bottom, Right.
		/// going with the above example this would be {0, 0, 2000, 4000}
		std::array<double, 4> m_Bounds = { 0, 0, 0, 0 };

		/// Store the affine transform as a geometric mesh, this is the intermediate
		/// step of going from the full res warp to the non-affine transformation.
		/// Unlike the non-affine transform these are in actual real-world coordinates
		/// such that the quad drawn by this transform represents (minus the non-affine 
		/// transform) the actual output location.
		/// This transform accounts for translation, scaling, rotation and skew.
		/// 
		/// an example of a scaled transform for 9000x4000 pixel image scaled down to 
		/// 4500x2000 would be:
		/// 
		/// { 0, 0    } { 4500, 0    }
		/// { 0, 2000 } { 4500, 2000 }
		/// 
		/// The points are stored in the order: top-left, top-right, bottom-left, bottom-right
		std::array<Geometry::Point2D<double>, 4> m_Transform;

		/// Store the non affine transform as a geometric mesh. 
		/// 
		/// Here we simply describe the shear/perspective warp applied to the mesh.
		/// 
		/// This transform is stored in a way that if it was a no op it would be made up of 
		/// 4 points that are identical to m_Transform. Continuing from the above example, a no op would
		/// look like this:
		/// 
		/// { 0, 0    } { 4500, 0    }
		/// { 0, 2000 } { 4500, 2000 }
		/// 
		/// Whereas an outwards perspective warp on the top edge could look like this:
		/// 
		/// { -500, 0 } { 5000, 0    }
		/// { 0, 2000 } { 4500, 2000 }
		/// 
		/// The points are stored in the order: top-left, top-right, bottom-left, bottom-right
		std::array<Geometry::Point2D<double>, 4> m_NonAffineTransform;

		size_t m_uDims = 4;
		size_t m_vDims = 4;

		WarpType m_WarpType = WarpType::normal;

		/// This will be "warpCustom" in all cases except for if we have a quilt warp in which case the 
		/// "warp" descriptor will have "warpNone" and the "warpQuilt" will have the actual warp. Another
		/// case where this would be none is for a puppet or smart perspective warp but those shouldn't
		/// reach here anyways
		std::string m_WarpStyle = "warpCustom";
		double m_WarpValue = 0.0f;
		double m_WarpPerspective = 0.0f;
		double m_WarpPerspectiveOther = 0.0f;

		std::string m_WarpRotate = "Hrzn";

		/// These appear to always be 4 no matter what the actual underlying structure looks like
		int32_t m_uOrder = 4;
		int32_t m_vOrder = 4;

		/// Only populated if we are dealing with a quilt warp
		std::vector<double> m_QuiltSlicesX;
		std::vector<double> m_QuiltSlicesY;

	private:

		/// Return the warp points with the transformations described by m_AffineTransform and m_NonAffineTransform applied to them
		std::vector<Geometry::Point2D<double>> get_transformed_source_points() const;

		/// Deserialize the common components between the quilt and normal warp
		static void _deserialize_common(Warp& warpStruct, const Descriptors::Descriptor& warpDescriptor);

		/// Validate the number of u and v dimensions making sure they form cubic bezier patches.
		static void validate_u_v_dims(size_t u_dimensions, size_t v_dimensions);

		// Overloads for serializing specific types
		Descriptors::Descriptor _serialize(quilt_warp) const;
		Descriptors::Descriptor _serialize(normal_warp) const;
		
	};

	/// Not yet supported
	struct PuppetWarp
	{
		// Not yet supported
	};


	/// Smart filter perspective warp, not yet supported
	struct PerspectiveWarp
	{
		// Not yet supported
	};

}

PSAPI_NAMESPACE_END
