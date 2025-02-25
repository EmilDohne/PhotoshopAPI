#include "LrSectionTaggedBlock.h"

#include "Core/FileIO/LengthMarkers.h"

PSAPI_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LrSectionTaggedBlock::read(File& document, const uint64_t offset, const Signature signature, const uint16_t padding)
{
	m_Key = Enum::TaggedBlockKey::lrSectionDivider;
	m_Offset = offset;
	m_Signature = signature;
	uint32_t length = ReadBinaryData<uint32_t>(document);
	length = RoundUpToMultiple<uint32_t>(length, padding);
	m_Length = length;

	uint32_t type = ReadBinaryData<uint32_t>(document);
	if (type < 0 || type > 3)
	{
		PSAPI_LOG_ERROR("TaggedBlock", "Layer Section Divider type has to be between 0 and 3, got %u instead", type);
	};
	auto sectionDividerType = Enum::getSectionDivider<uint32_t, Enum::SectionDivider>(type);
	if (sectionDividerType)
	{
		m_Type = sectionDividerType.value();
	}
	else
	{
		PSAPI_LOG_ERROR("TaggedBlock", "Could not find Layer Section Divider type by value");
	}


	// This overrides the layer blend mode if it is present.
	if (length >= 12u)
	{
		Signature sig = Signature(ReadBinaryData<uint32_t>(document));
		if (sig != Signature("8BIM"))
		{
			PSAPI_LOG_ERROR("TaggedBlock", "Signature does not match '8BIM', got '%s' instead",
				uint32ToString(sig.m_Value).c_str());
		}

		std::string blendModeStr = uint32ToString(ReadBinaryData<uint32_t>(document));
		m_BlendMode = Enum::getBlendMode<std::string, Enum::BlendMode>(blendModeStr);
	}

	if (length >= 16u)
	{
		// This is the sub-type information, probably for animated photoshop files
		// we do not care about this currently
		document.skip(4u);
	}
};


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void LrSectionTaggedBlock::write(File& document, [[maybe_unused]] const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, [[maybe_unused]] const uint16_t padding /*= 1u*/)
{
	WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
	WriteBinaryData<uint32_t>(document, Signature("lsct").m_Value);
	Impl::ScopedLengthBlock<uint32_t> len_block(document, padding);

	auto sectionDividerType = Enum::getSectionDivider<Enum::SectionDivider, uint32_t>(m_Type);
	if (sectionDividerType)
	{
		WriteBinaryData<uint32_t>(document, sectionDividerType.value());
	}
	else
	{
		PSAPI_LOG_ERROR("TaggedBlock", "Could not find Layer Section Divider type by value");
	}

	// For some reason the blend mode has another 4 bytes for a 8BIM key
	if (m_BlendMode.has_value())
	{
		WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
		std::optional<std::string> blendModeStr = Enum::getBlendMode<Enum::BlendMode, std::string>(m_BlendMode.value());
		if (!blendModeStr.has_value())
			PSAPI_LOG_ERROR("LayerRecord", "Could not identify a blend mode string from the given key");
		else
			WriteBinaryData<uint32_t>(document, Signature(blendModeStr.value()).m_Value);
	}

	// There is an additional variable here for storing information related to timelines, but seeing as we do not care 
	// about animated PhotoshopFiles at this moment we dont write anything here
}

PSAPI_NAMESPACE_END