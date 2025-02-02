#pragma once

#include "Macros.h"
#include "PhotoshopFile/FileHeader.h"

#include <cmath>

PSAPI_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
struct ChannelCoordinates
{
	int32_t width;
	int32_t height;
	float centerX;
	float centerY;

	ChannelCoordinates() = default;
	ChannelCoordinates(int32_t _width, int32_t _height, float _centerX, float _centerY) : width(_width), height(_height), centerX(_centerX), centerY(_centerY) {};
};


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
struct ChannelExtents
{
	int32_t top;
	int32_t left;
	int32_t bottom;
	int32_t right;

	ChannelExtents() = default;
	ChannelExtents(int32_t _top, int32_t _left, int32_t _bottom, int32_t _right) : top(_top), left(_left), bottom(_bottom), right(_right) {};
};


/// Generate Channel Coordinates as we use them in the LayeredFile from Channel Extents as present in Photoshop documents
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
inline ChannelCoordinates generateChannelCoordinates(const ChannelExtents extents)
{
	ChannelCoordinates coords = {};
	// Generate our coordinates from the layer extents, for the x and y coordinates we calculate the offset of the centers
	// and use that 
	coords.width = extents.right - extents.left;
	coords.height = extents.bottom - extents.top;

	coords.centerX = static_cast<float>(extents.right + extents.left) / 2;
	coords.centerY = static_cast<float>(extents.bottom + extents.top) / 2;

	return coords;
}


/// Generate Channel Coordinates as we use them in the LayeredFile from Channel Extents as present in Photoshop documents
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
inline ChannelExtents generate_extents(const ChannelCoordinates coordinates)
{
	ChannelExtents extents = {};

	extents.top = static_cast<int32_t>(coordinates.centerY - .5 * coordinates.width);
	extents.left = static_cast<int32_t>(coordinates.centerX - .5 * coordinates.width);
	extents.bottom = extents.top + coordinates.height;
	extents.right = extents.left + coordinates.width;
	
	return extents;
}

PSAPI_NAMESPACE_END