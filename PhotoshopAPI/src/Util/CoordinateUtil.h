#pragma once

#include "Macros.h"
#include "PhotoshopFile/FileHeader.h"

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
inline ChannelCoordinates generateChannelCoordinates(const ChannelExtents extents, const FileHeader& header)
{
	ChannelCoordinates coords = {};
	// Generate our coordinates from the layer extents, for the x and y coordinates we calculate the offset of the centers
	// and use that 
	coords.width = extents.right - extents.left;
	coords.height = extents.bottom - extents.top;

	// Documents start at 0, 0 and goes to width, height.
	// We need floats here as when dividing by 2 we would otherwise truncate
	float documentCenterX = static_cast<float>(header.m_Width) / 2;
	float documentCenterY = static_cast<float>(header.m_Height) / 2;

	// Calculate our layer coordinates by adding half the width to the left
	float layerCenterX = static_cast<float>(extents.left) + static_cast<float>(coords.width) / 2;
	float layerCenterY = static_cast<float>(extents.top) + static_cast<float>(coords.height) / 2;

	// Finally just calculate the difference between these two
	coords.centerX = layerCenterX - documentCenterX;
	coords.centerY = layerCenterY - documentCenterY;

	return coords;
}


/// Generate Channel Coordinates as we use them in the LayeredFile from Channel Extents as present in Photoshop documents
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
inline ChannelExtents generateChannelExtents(const ChannelCoordinates coordinates, const FileHeader& header)
{
	ChannelExtents extents = {};

	// The document always uses 0 based extents. so if a document is 64x64 pixels the extents would be 
	// [0, 0, 64, 64] making our calculations much easier
	int32_t documentTop = 0;
	int32_t documentLeft = 0;
	int32_t documentBottom = header.m_Height;
	int32_t documentRight = header.m_Width;

	// Our center coordinates are relative to in the middle of the canvas, which means if continuing our 
	// example they translate to 32, 32

	float translatedCenterX = static_cast<float>(documentRight) / 2 + coordinates.centerX;
	float translatedCenterY = static_cast<float>(documentBottom) / 2 + coordinates.centerY;

	// Use our translated center variables to make Photoshop compliant coordinates. If the 
	// image was also 64x64 pixels this would then create these extents [0, 0, 64, 64]

	extents.top = static_cast<int32_t>(translatedCenterY - static_cast<float>(coordinates.height) / 2);
	extents.left = static_cast<int32_t>(translatedCenterX - static_cast<float>(coordinates.width) / 2);
	extents.bottom = static_cast<int32_t>(translatedCenterY + static_cast<float>(coordinates.height) / 2);
	extents.right = static_cast<int32_t>(translatedCenterX + static_cast<float>(coordinates.width) / 2);
	
	return extents;
}

PSAPI_NAMESPACE_END