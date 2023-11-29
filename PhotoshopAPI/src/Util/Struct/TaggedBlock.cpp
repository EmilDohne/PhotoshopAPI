#include "TaggedBlock.h"

#include "Macros.h"
#include "Enum.h"
#include "Logger.h"
#include "StringUtil.h"
#include "Struct/Signature.h"
#include "Struct/File.h"
#include "PhotoshopFile/FileHeader.h"

PSAPI_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TaggedBlock::TaggedBlock(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const Enum::TaggedBlockKey key, const uint16_t padding)
{
	m_Offset = offset;
	m_Signature = signature;
	m_Key = key;
	if (Enum::isTaggedBlockSizeUint64(m_Key) && header.m_Version == Enum::Version::Psb)
	{
		uint64_t length = ReadBinaryData<uint64_t>(document);
		length = RoundUpToMultiple<uint64_t>(length, padding);
		m_Length = length;
		document.skip(length);

		m_TotalLength = length + 4u + 4u + 8u;
	}
	else
	{
		uint32_t length = ReadBinaryData<uint32_t>(document);
		length = RoundUpToMultiple<uint32_t>(length, padding);
		m_Length = length;
		document.skip(length);


		m_TotalLength = static_cast<uint64_t>(length) + 4u + 4u + 4u;
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
Lr16TaggedBlock::Lr16TaggedBlock(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const uint16_t padding) 
{
	m_Key = Enum::TaggedBlockKey::Lr16;
	m_Offset = offset;
	m_Signature = signature;
	uint64_t length = ExtractWidestValue<uint32_t, uint64_t>(ReadBinaryDataVariadic<uint32_t, uint64_t>(document, header.m_Version));
	length = RoundUpToMultiple<uint64_t>((length), padding);
	m_Length = length;
	m_Data = LayerInfo(document, header, document.getOffset(), true, std::get<uint64_t>(m_Length));

	m_TotalLength = length + 4u + 4u + 8u;
};


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
Lr32TaggedBlock::Lr32TaggedBlock(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const uint16_t padding)
{
	m_Key = Enum::TaggedBlockKey::Lr32;
	m_Offset = offset;
	m_Signature = signature;
	uint64_t length = ExtractWidestValue<uint32_t, uint64_t>(ReadBinaryDataVariadic<uint32_t, uint64_t>(document, header.m_Version));
	length = RoundUpToMultiple<uint64_t>((length), padding);
	m_Length = length;
	m_Data = LayerInfo(document, header, document.getOffset(), true, std::get<uint64_t>(m_Length));

	m_TotalLength = length + 4u + 4u + 8u;
};


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
LrSectionTaggedBlock::LrSectionTaggedBlock(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const uint16_t padding)
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
		PSAPI_LOG_ERROR("TaggedBlock", "Layer Section Divider type has to be between 0 and 3, got %u instead", type)
	};
	m_Type = Enum::sectionDividerMap.at(type);


	// This overrides the layer blend mode if it is present.
	if (length >= 12u)
	{
		Signature sig = Signature(ReadBinaryData<uint32_t>(document));
		if (sig != Signature("8BIM"))
		{
			PSAPI_LOG_ERROR("TaggedBlock", "Signature does not match '8BIM', got '%s' instead",
				uint32ToString(sig.m_Value).c_str())
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

	m_TotalLength = static_cast<uint64_t>(length) + 4u + 4u + 4u;
};

PSAPI_NAMESPACE_END

