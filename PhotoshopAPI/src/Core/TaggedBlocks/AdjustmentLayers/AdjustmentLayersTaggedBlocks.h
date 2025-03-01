#pragma once

#include "Macros.h"
#include "Core/TaggedBlocks/TaggedBlock.h"

#include "Core/Struct/File.h"
#include "Core/Struct/Signature.h"
#include "Core/Struct/DescriptorStructure.h"
#include "Core/Struct/UnicodeString.h"
#include "Util/Enum.h"
#include "Util/Logger.h"

#include <array>
#include <vector>


PSAPI_NAMESPACE_BEGIN

// Uses 'CgEd' block instead of 'brit', 'brit' is apparently legacy
struct BrightnessContrastTaggedBlock : TaggedBlock
{
	Descriptors::Descriptor m_Descriptor{};

	void read(File& document, const uint64_t offset, const Signature signature, const uint16_t padding = 1u);
};


struct LevelRecord
{
	int16_t input_floor = 0; // 0-253
	int16_t input_ceiling = 255; // 2-255
	int16_t output_floor = 0; // matched to input_floor
	int16_t output_ceiling = 255; // 0-255
	float gamma = 1.0; // from 0.1 - 9.99

	inline static LevelRecord read(File& document)
	{
		LevelRecord record{};

		record.input_floor = ReadBinaryData<int16_t>(document);
		record.input_ceiling = ReadBinaryData<int16_t>(document);
		record.output_floor = ReadBinaryData<int16_t>(document);
		record.output_ceiling = ReadBinaryData<int16_t>(document);
		auto gamma = ReadBinaryData<int16_t>(document);
		record.gamma = gamma / 10.0f;

		return record;
	}
};

struct LevelsTaggedBlock : TaggedBlock
{
	/// Level record sets order
	/// The first set of levels is the master set that applies to all of the composite channels(RGB) when in composite image mode.
	/// The remaining sets apply to the active channels individually; set two applies to channel one, the set three to channel two, etc., up until set 25, which applies to channel 24.
	/// Sets 28 and 29 are reserved and should be set to zeros.
	/// 
	/// Indexed color
	/// The exception to the normal order is when the mode is Indexed :
	/// The first set is a master set.
	/// The next three sets are created for the Red, Green, and Blue portions of the image's color table, and they are applied to the first channel.
	/// The remaining sets apply to any remaining alpha channels that are active : for instance, if channel two is active, set five applies to it; if channel three is active, set six applies to it, etc., up until channel 27, which applies to channel 24.
	/// Sets 28 and 29 are reserved and should be set to zeros.
	/// 
	/// Single active channels
	/// Photoshop handles single active channels in a special fashion.When saving the levels applied to a single channel, the settings are stored into the master set, at the beginning of the file.Similarly, when reading a levels file for application to a single active channel, the master levels are the ones that will be used on that channel.This allows easy application of a single file to both RGB and grayscale images.
	std::array<LevelRecord, 29> m_LevelRecords{};
	std::vector<LevelRecord> m_ExtraLevelRecords{};

	void read(File& document, const uint64_t offset, const Signature signature, const uint16_t padding = 1u);
};

struct CurvesTaggedBlock : TaggedBlock
{

	void read(File& document, const uint64_t offset, const Signature signature, const uint16_t padding = 1u);
};


PSAPI_NAMESPACE_END