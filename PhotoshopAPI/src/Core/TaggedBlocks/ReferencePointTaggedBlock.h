#pragma once

#include "Macros.h"
#include "TaggedBlock.h"

#include "Core/Struct/File.h"
#include "Core/Struct/Signature.h"
#include "Util/ProgressCallback.h"
#include "Util/Enum.h"
#include "Util/Logger.h"

PSAPI_NAMESPACE_BEGIN

/// This Tagged block appears to store information about how the layer is tranformed (flipped rotated etc.) The "Reference Point" as it is called stores the absolute world 
/// location of what the top left pixel would be. That means if we have a layer with an imaginary extent of [16, 16, 48, 48] (the scene size does not matter).
/// a reference point of (48.0f, 16.0f) would tell us the top left of the image is actually currently at the top right extents which would relate to a horizontal flip.
/// conversely a reference point of (16.0f, 48.0f) would relate to a vertical flip. A flip on both axis would be (48.0f, 48.0f).
/// 
/// Unfortunately it is currently unclear how photoshop distinguishes between rotations and flips as a 90 degree turn clockwise (which does not look the same as a horizontal
/// flip) relates to the same reference point as a horizontal flip (48.0f, 16.0f).
/// 
/// For 90 degree turns for example the Reference point coordinates are the same as a horizontal flip (48.0f, 16.0f) but the actual image data is rotated
/// 
/// Due to this uncertain behaviour this block is only for roundtripping for the time being
struct ReferencePointTaggedBlock : TaggedBlock
{
	/// The absolute X Coordinate reference point for transforms, this must be within the bounding box
	/// of the layer (or less than .5 pixels away as bbox is stored in float while 
	double m_ReferenceX = 0.0f;
	// The absolute Y Coordinate reference point for transforms
	double m_ReferenceY = 0.0f;

	ReferencePointTaggedBlock() = default;
	ReferencePointTaggedBlock(double refX, double refY) : m_ReferenceX(refX), m_ReferenceY(refY) {};

	void read(File& document, const uint64_t offset, const Signature signature);
	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u) override;
};


PSAPI_NAMESPACE_END
