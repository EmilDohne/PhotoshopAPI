#pragma once

#include "Macros.h"

#include "Core/Struct/DescriptorStructure.h"

#include "Core/Render/Render.h"
#include "Core/Render/ImageBuffer.h"
#include "Core/Geometry/Point.h"
#include "Core/Geometry/Mesh.h"
#include "Core/Geometry/BezierSurface.h"


#include <array>


PSAPI_NAMESPACE_BEGIN

namespace SmartObject
{

	/// Base warp structure, these encompass the warps found under Edit -> Transform which are:
	/// 
	/// Edit/Transform/Skew
	/// Edit/Transform/Distort
	/// Edit/Transform/Perspective
	/// Edit/Transform/Warp
	/// 
	/// These are stored as part of the descriptor found on the PlacedLayer or PlacedLayerData Tagged Blocks.
	/// The data stored on the file does not differentiate between these types and they are all stored
	/// identically
	struct Warp
	{
		// Tags for dispatch
		struct normal_warp {};
		struct quilt_warp {};

		enum class WarpType
		{
			normal,	// Normal warps in this case define a 4x4 warp grid
			quilt,	// Quilt warps are for arbitrary warp resolutions
		};

		Warp() = default;

		/// Initialize the warp struct from a set of geometric points describing a bezier surface 
		/// one or more quadratic bezier patches. These points are in scanline order (i.e. going first along the horizontal
		/// axis, then across the vertical axis). 
		/// Being a set of quadratic bezier patches the dimensions across the u and v (x and y) must be `4` or `4 + n * 3`
		/// where `n` is the number of subdivisions and is greater than one. In simple terms this means a valid number of 
		/// points per axis is 4, 7, 10, 13 etc.
		/// 
		/// \param warp The warp points in scanline order.
		/// \param u_dims The dimensions across the u (x)
		/// \param v_dims The dimensions across the v (y)
		Warp(std::vector<Geometry::Point2D<double>> warp, size_t u_dims, size_t v_dims)
		{
			if (warp.size() != u_dims * v_dims)
			{
				PSAPI_LOG_ERROR("SmartObjectWarp", "Number of u * v dimensions must match size of the warp structure, expected %zu but got {%zu, %zu}",
					warp.size(), u_dims, v_dims);
			}
			if (u_dims < 4 || v_dims < 4)
			{
				PSAPI_LOG_ERROR("SmartObjectWarp", "Unable to create smart object warp as its bezier surface is not at least cubic. Expected at the very least 4x4 divisions");
			}

			if (u_dims == 4 && v_dims == 4)
			{
				m_WarpType = WarpType::normal;
			}
			else
			{
				if ((static_cast<int>(u_dims) - 4) % 3 != 0)
				{
					PSAPI_LOG_ERROR("SmartObjectWarp",
						"Number of u dimensions would not lead to a quadratic bezier patch, received %zu dimensions but expected 4 + n * 3 dimensions (4, 7, 10, 13...)", u_dims);
				}
				if ((static_cast<int>(v_dims) - 4) % 3 != 0)
				{
					PSAPI_LOG_ERROR("SmartObjectWarp",
						"Number of v dimensions would not lead to a quadratic bezier patch, received %zu dimensions but expected 4 + n * 3 dimensions (4, 7, 10, 13...)", v_dims);
				}
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
		}

		/// Check if the warp struct is valid, for now returns whether the warp points hold any data
		bool valid() { return m_WarpPoints.size() > 0; }

		/// Generate a Mesh from our warp structure, this is primarily used for directly
		/// visualizing the points. 
		Geometry::Mesh<double> mesh() const;

		/// Generate a bezier surface from this warp structure. This can be used e.g. for rendering
		Geometry::BezierSurface surface() const;

		/// Apply the warp by warping the `image` into the `buffer` using the locally stored warp description. 
		/// The `buffer` passed should match the general resolution of the warp points. So if e.g. the warp points 
		/// are from { 0 - 4000, 0 - 2000 } the `buffer` parameter should cover these.
		/// If this isn't the case the function won't fail but the image will not contain the full warped picture.
		/// 
		/// \param buffer The buffer to render into
		/// \param image  The image do warp using the local warp struct
		/// \param warp_mesh  A mesh to apply the warp with, mainly to be used as an optimization step if 
		///					  you wish to apply the warp to multiple channels at the same time to only calculate the 
		///				      mesh construction once. Can be gotten using `SmartObjectWarp::mesh()`
		/// \param resolution The resolution of warp geometry. Since we first convert the BezierSurface into a Mesh 
		///					  this parameter dictates how many pixels apart each subdivision line should be. Do note
		///					  that increasing this massively does not necessarily give a better result. Defaults to `25`.
		template <typename T>
		void apply(Render::ImageBuffer<T> buffer, Render::ImageBuffer<T, true> image, const Geometry::Mesh<double>& warp_mesh, size_t resolution = 50) const
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
					PSAPI_PROFILE_SCOPE("Warp scanline");
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
						if (uv == static_cast<double>(-1.0f))
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
		/// \param buffer The buffer to render into
		/// \param image  The image do warp using the local warp struct
		/// \param resolution The resolution of warp geometry. Since we first convert the BezierSurface into a Mesh 
		///					  this parameter dictates how many pixels apart each subdivision line should be. Do note
		///					  that increasing this massively does not necessarily give a better result. Defaults to `25`.
		template <typename T>
		void apply(Render::ImageBuffer<T> buffer, Render::ImageBuffer<T, true> image, size_t resolution = 50) const
		{
			auto warp_surface = surface();
			auto warp_mesh = warp_surface.mesh(buffer.width / resolution, buffer.height / resolution, m_NonAffineTransform);
			apply(buffer, image, warp_mesh, resolution);
		}

		/// Create a "warp" descriptor from this class ready to be stored on a PlacedLayer or PlacedLayerData tagged block
		Descriptors::Descriptor serialize() const;

		/// Deserialize the SmartObjectWarp from a warp descriptor. In the context 
		/// of the smart object this would be at the "warp" key. has specializations 
		static Warp deserialize(const Descriptors::Descriptor& warp_descriptor, const Descriptors::List& transform, const Descriptors::List& non_affine_transform, normal_warp);
		static Warp deserialize(const Descriptors::Descriptor& quilt_warp_descriptor, const Descriptors::List& transform, const Descriptors::List& non_affine_transform, quilt_warp);

		inline size_t u_dimensions() const noexcept { return m_uDims; };
		inline size_t v_dimensions() const noexcept { return m_vDims; };

		// Get the bounds of the smart object warp, defined as {top, left, bottom, right}
		inline std::array<double, 4> bounds() const { return m_Bounds; }

		static std::array<Geometry::Point2D<double>, 4> generate_non_affine_mesh(const Descriptors::List& transform, const Descriptors::List& non_affine_transform);

		/// Setters, usually wont have to touch these as they are for roundtripping
		void non_affine_mesh(std::array<Geometry::Point2D<double>, 4> non_affine_transform_mesh);
		std::array<Geometry::Point2D<double>, 4> non_affine_mesh() const;
		void warp_style(std::string style);
		void warp_value(double value);
		void warp_perspective(double value);
		void warp_perspective_other(double value);
		void warp_rotate(std::string rotate);

		void quilt_slices_x(std::vector<double> slices);
		void quilt_slices_y(std::vector<double> slices);

		void warp_type(WarpType type);

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

		/// Store the non affine transform as a geometric mesh. This transform
		/// is stored in a way that if it was a no op it would be made up of 
		/// 4 points that just describe a square. Therefore all transforms are 
		/// relative to this 1x1 square. Below you can see what a no op would look
		/// like
		/// { 0, 0 } { 1, 0 }
		/// { 0, 1 } { 1, 1 }
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

		/// Deserialize the common components between the quilt and normal warp
		static void deserializeCommon(Warp& warpStruct, const Descriptors::Descriptor& warpDescriptor);
	};


	struct PuppetWarp
	{
		// Not yet supported
	};


	/// Smart filter perspective warp
	struct PerspectiveWarp
	{
		// Not yet supported
	};

}

PSAPI_NAMESPACE_END
