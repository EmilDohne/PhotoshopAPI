#pragma once

#include "Macros.h"

#include "Core/Struct/DescriptorStructure.h"

#include "Core/Render/Render.h"
#include "Core/Geometry/Point.h"
#include "Core/Geometry/Mesh.h"
#include "Core/Geometry/BezierSurface.h"


#include <array>


PSAPI_NAMESPACE_BEGIN


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
struct SmartObjectWarp
{

	enum class WarpType
	{
		normal,	// Normal warps in this case define a 4x4 warp grid
		quilt	// Quilt warps are for arbitrary warp resolutions
	};


	/// The warp points, this is a coordinate grid of u*v points going in scanline order from the top left 
	/// to the bottom right, here an example for a 4x4 grid
	/// 1  2  3  4
	/// 5  6  7  8
	/// 9  10 11 12
	/// 13 14 15 16
	/// 
	/// The points themselves don't describe coordinates on the canvas but instead on the resolution
	/// of the smart object. So if the canvas is 1000x500 pixels but the smart object's original resolution
	/// is 4000x2000 the points are the offsets from that. These points represent the control points of a bezier
	/// surface which we can construct from 
	std::vector<Geometry::Point2D<double>> m_WarpPoints;

	/// The bounds of the geometry. Defined as Top, Left, Bottom, Right.
	/// going with the above example this would be {0, 0, 2000, 4000}
	std::array<double, 4> m_Bounds = {0, 0, 0, 0};

	SmartObjectWarp() = default;
	SmartObjectWarp(std::vector<Geometry::Point2D<double>> warp, size_t uDims, size_t vDims, std::array<double, 4> bounds) 
		: m_Bounds(bounds), m_WarpPoints(warp), m_uDims(uDims), m_vDims(vDims) 
	{
		if (warp.size() != uDims * vDims)
		{
			PSAPI_LOG_ERROR("SmartObjectWarp", "Number of u * v dimensions must match size of the warp structure, expected %zu but got {%zu, %zu}",
				warp.size(), uDims, vDims);
		}

		if (uDims == 4 && vDims == 4)
		{
			m_WarpType = WarpType::normal;
		}
		else
		{
			m_WarpType = WarpType::quilt;
		}
	}

	/// Construct a default warp from the given extents, will raise an error if the initial
	/// rect is not orthogonal. Defined as top_left, top_rigth, bottom_left, bottom_right
	SmartObjectWarp(std::array<Geometry::Point2D<double>, 4> rect);
	SmartObjectWarp(Geometry::Point2D<double> topLeft, Geometry::Point2D<double> topRight, Geometry::Point2D<double> bottomLeft, Geometry::Point2D<double> bottomRight);

	/// Generate a Mesh from our warp structure, this is primarily used for directly
	/// visualizing the points. 
	Geometry::Mesh<double> mesh() const;

	/// Generate a bezier surface from this warp structure. This can be used e.g. for rendering
	Geometry::BezierSurface surface() const;

	/// Create a "warp" descriptor from this class ready to be stored on a PlacedLayer or PlacedLayerData tagged block
	Descriptors::Descriptor serialize() const;

	/// Apply the warp by warping the `image` into the `buffer` using the locally stored warp description. 
	/// The `buffer` passed should match the general resolution of the warp points. So if e.g. the warp points 
	/// are from { 0 - 4000, 0 - 2000 } the `buffer` parameter should cover these.
	/// If this isn't the case the function won't fail but the image will not contain the full warped picture.
	/// 
	/// \param buffer The buffer to render into
	/// \param image  The image do warp using the local warp struct
	/// \param resolution The resolution of warp geometry. Since we first convert the BezierSurface into a Mesh 
	///					  this parameter dictates how many pixels apart each subdivision line should be. Do note
	///					  that increasing this massively does not necessarily give a better result.
	template <typename T>
	void apply(Render::ImageBuffer<T> buffer, Render::ImageBuffer<T, true> image, size_t resolution = 25) const
	{
		auto warp_surface = surface();
		auto warp_mesh = warp_surface.mesh(buffer.width / resolution, buffer.height / resolution);

		std::vector<size_t> vertical_iter(buffer.height);
		std::iota(vertical_iter.begin(), vertical_iter.end(), 0);

		std::for_each(std::execution::par_unseq, vertical_iter.begin(), vertical_iter.end(), [&](size_t y)
			{
				for (size_t x = 0; x < buffer.width; ++x)
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

	/// Deserialize the SmartObjectWarp from a warp descriptor. In the context 
	/// of the smart object this would be at the "warp" key
	static SmartObjectWarp deserialize(const Descriptors::Descriptor& warpDescriptor, WarpType type);

	/// Setters, usually wont have to touch these as they are for roundtripping
	void warp_style(std::string style);
	void warp_value(double value);
	void warp_perspective(double value);
	void warp_perspective_other(double value);
	void warp_rotate(std::string rotate);

	void quilt_slices_x(std::vector<double> slices);
	void quilt_slices_y(std::vector<double> slices);

private:
	size_t m_uDims = 4;
	size_t m_vDims = 4;

	WarpType m_WarpType				= WarpType::normal;

	/// This will be "warpCustom" in all cases except for if we have a quilt warp in which case the 
	/// "warp" descriptor will have "warpNone" and the "warpQuilt" will have the actual warp. Another
	/// case where this would be none is for a puppet or smart perspective warp but those shouldn't
	/// reach here anyways
	std::string m_WarpStyle			= "warpCustom";
	double m_WarpValue				= 0.0f;
	double m_WarpPerspective		= 0.0f;
	double m_WarpPerspectiveOther	= 0.0f;

	std::string m_WarpRotate = "Hrzn";

	/// These appear to always be 4 no matter what the actual underlying structure looks like
	int32_t m_uOrder = 4;
	int32_t m_vOrder = 4;

	/// Only populated if we are dealing with a quilt warp
	std::vector<double> m_QuiltSlicesX;
	std::vector<double> m_QuiltSlicesY;

	/// Photoshop recognizes 2 different types of warps, "Normal" is a simple 4x4 cubic 
	/// bezier while a "Quilt" is a warp with m*n dimensions. We still evaluate both a 
	/// cubic bezier patch
	static SmartObjectWarp deserializeQuilt(const Descriptors::Descriptor& warpDescriptor);
	static SmartObjectWarp deserializeNormal(const Descriptors::Descriptor& warpDescriptor);

	/// Deserialize the common components between the quilt and normal warp
	static void deserializeCommon(SmartObjectWarp& warpStruct, const Descriptors::Descriptor& warpDescriptor);
};


/// The "Smart Filter" Puppet warp that gets applied live onto the 
struct SmartObjectPuppetWarp
{

};


struct SmartObjectPerspectiveWarp
{
};


PSAPI_NAMESPACE_END
