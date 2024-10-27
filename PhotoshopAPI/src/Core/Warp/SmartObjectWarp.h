#pragma once

#include "Macros.h"

#include "Core/Struct/DescriptorStructure.h"
#include "Core/Struct/Point.h"


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

	/// The warp points, this is a coordinate grid of 16 points going in scanline order from the top left 
	/// to the bottom right
	/// 1  2  3  4
	/// 5  6  7  8
	/// 9  10 11 12
	/// 13 14 15 16
	/// 
	/// The points themselves don't describe coordinates on the canvas but instead on the resolution
	/// of the smart object. So if the canvas is 1000x500 pixels but the smart object's original resolution
	/// is 4000x2000 the points are the offsets from that. 
	std::array<Point<double>, 16> m_WarpPoints;

	/// The bounds of the geometry. Defined as Top, Left, Bottom, Right.
	/// going with the above example this would be {0, 0, 2000, 4000}
	std::array<double, 4> m_Bounds;

	SmartObjectWarp() = default;
	SmartObjectWarp(std::array<Point<double>, 16> warp, std::array<double, 4> bounds) : m_Bounds(bounds), m_WarpPoints(warp) {}

	/// Construct a default warp from the given extents, will raise an error if the initial
	/// rect is not orthogonal. Defined as top_left, top_rigth, bottom_left, bottom_right
	SmartObjectWarp(std::array<Point<double>, 4> rect);

	SmartObjectWarp(Point<double> topLeft, Point<double> topRight, Point<double> bottomLeft, Point<double> bottomRight);


	/// Create a "warp" descriptor from this class ready to be stored on a PlacedLayer or PlacedLayerData tagged block
	Descriptors::Descriptor serialize() const;
	/// Deserialize the SmartObjectWarp from a warp descriptor. In the context 
	/// of the smart object this would be at the "warp" key
	static SmartObjectWarp deserialize(const Descriptors::Descriptor& warpDescriptor);

private:

	std::string m_WarpStyle			= "warpCustom";
	double m_WarpValue				= 0.0f;
	double m_WarpPerspective		= 0.0f;
	double m_WarpPerspectiveOther	= 0.0f;

	std::string m_WarpRotate;

	int32_t m_uOrder = 4;
	int32_t m_vOrder = 4;
};


/// The "Smart Filter" Puppet warp that gets applied live onto the 
struct SmartObjectPuppetWarp
{

};


struct SmartObjectPerspectiveWarp
{
};


PSAPI_NAMESPACE_END
