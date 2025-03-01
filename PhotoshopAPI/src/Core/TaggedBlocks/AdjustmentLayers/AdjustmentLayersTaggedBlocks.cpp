#include "AdjustmentLayersTaggedBlocks.h"

#include "Core/FileIO/LengthMarkers.h"
#include "Util/StringUtil.h"

#include <ranges>

PSAPI_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void BrightnessContrastTaggedBlock::read(File& document, const uint64_t offset, const Signature signature, const uint16_t padding = 1u)
{
	m_Key = Enum::TaggedBlockKey::adjBrightnessContrastNew;
	m_Offset = offset;
	m_Signature = signature;
	uint32_t length = ReadBinaryData<uint32_t>(document);
	length = RoundUpToMultiple<uint32_t>(length, padding);
	m_Length = length;

	auto start_offset = document.get_offset();

	m_Descriptor.read(document);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LevelsTaggedBlock::read(File& document, const uint64_t offset, const Signature signature, const uint16_t padding = 1u)
{
	m_Key = Enum::TaggedBlockKey::adjBrightnessContrastNew;
	m_Offset = offset;
	m_Signature = signature;
	uint32_t length = ReadBinaryData<uint32_t>(document);
	length = RoundUpToMultiple<uint32_t>(length, padding);
	m_Length = length;

	auto start_offset = document.get_offset();

	auto version = ReadBinaryData<uint16_t>(document);
	if (version != 2)
	{
		PSAPI_LOG_ERROR("LevelsTaggedBlock", "Invalid version encountered, expected 2 but instead got %d", static_cast<int>(version));
	}
	// Read the normal level records
	for (auto idx : std::views::iota(0, 29))
	{
		m_LevelRecords[idx] = LevelRecord::read(document);
	}

	// These are the extra level records
	if (static_cast<int>(length) - (document.get_offset() - start_offset) >= 6)
	{
		Signature sig = Signature::read(document);
		auto extra_version = ReadBinaryData<uint16_t>(document);

		if (sig != "Lvls")
		{
			PSAPI_LOG_ERROR(
				"LevelsTaggedBlock", 
				"Invalid signature encountered, expected 'lvls' but instead got %c%c%c%c", 
				sig.m_Representation[0],
				sig.m_Representation[1],
				sig.m_Representation[2],
				sig.m_Representation[3]
			);
		}
		if (extra_version != 3)
		{
			PSAPI_LOG_ERROR("LevelsTaggedBlock", "Invalid extra_version encountered, expected 3 but instead got %d", static_cast<int>(extra_version));
		}

		auto count = static_cast<int>(ReadBinaryData<uint16_t>(document)) - 29;
		for (auto idx : std::views::iota(0, count))
		{
			m_ExtraLevelRecords.push_back(LevelRecord::read(document));
		}
	}

}

PSAPI_NAMESPACE_END