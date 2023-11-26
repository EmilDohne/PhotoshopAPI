#include "TaggedBlock.h"

#include "Macros.h"
#include "Enum.h"
#include "Logger.h"
#include "StringUtil.h"
#include "Struct/Signature.h"
#include "Struct/File.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "PhotoshopFile/FileHeader.h"

PSAPI_NAMESPACE_BEGIN

// Unfortunately we must specify these tagged blocks here as they rely on the LayerAndMaskInformation which would otherwise lead to circular dependencies
namespace TaggedBlock
{

	// 16-bit files store this tagged block at the end of the layer and mask information section which contains the 
	// layer info section
	struct Lr16 : Base
	{
		LayerInfo m_Data;

		Lr16(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const uint16_t padding = 1u);
	};


	// 32-bit files store this tagged block at the end of the layer and mask information section which contains the 
	// layer info section
	struct Lr32 : Base
	{
		LayerInfo m_Data;

		Lr32(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const uint16_t padding = 1u);
	};
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TaggedBlock::Generic::Generic(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const Enum::TaggedBlockKey key, const uint16_t padding)
{
	m_Offset = offset;
	m_Signature = signature;
	m_Key = key;
	if (Enum::isTaggedBlockSizeUint64(m_Key) && header.m_Version == Enum::Version::Psb)
	{
		uint64_t length = ReadBinaryData<uint64_t>(document);
		length = RoundUpToMultiple<uint64_t>(length, padding);
		m_Length = length;
		m_Data = ReadBinaryArray<uint8_t>(document, std::get<uint64_t>(m_Length));

		m_TotalLength = length + 4u + 4u + 8u;
	}
	else
	{
		uint32_t length = ReadBinaryData<uint32_t>(document);
		length = RoundUpToMultiple<uint32_t>(length, padding);
		m_Length = length;
		m_Data = ReadBinaryArray<uint8_t>(document, std::get<uint32_t>(m_Length));

		m_TotalLength = static_cast<uint64_t>(length) + 4u + 4u + 4u;
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TaggedBlock::Lr16::Lr16(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const uint16_t padding)
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
TaggedBlock::Lr32::Lr32(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const uint16_t padding)
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
TaggedBlock::LayerSectionDivider::LayerSectionDivider(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const uint16_t padding)
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


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<TaggedBlock::Base> readTaggedBlock(File& document, const FileHeader& header, const uint16_t padding)
{
	const uint64_t offset = document.getOffset();
	Signature signature = Signature(ReadBinaryData<uint32_t>(document));
	if (signature != Signature("8BIM") && signature != Signature("8B64"))
	{
		PSAPI_LOG_ERROR("LayerRecord", "Signature does not match '8BIM' or '8B64', got '%s' instead",
			uint32ToString(signature.m_Value).c_str())
	}
	std::string keyStr = uint32ToString(ReadBinaryData<uint32_t>(document));
	std::optional<Enum::TaggedBlockKey> taggedBlock = Enum::getTaggedBlockKey<std::string, Enum::TaggedBlockKey>(keyStr);
	
	if (taggedBlock.has_value())
	{
		switch (taggedBlock.value())
		{
		case Enum::TaggedBlockKey::Lr16:
			return std::make_shared<TaggedBlock::Lr16>(document, header, offset, signature, padding);
		case Enum::TaggedBlockKey::Lr32:
			return std::make_shared<TaggedBlock::Lr32>(document, header, offset, signature, padding);
		case Enum::TaggedBlockKey::lrSectionDivider:
			return std::make_shared<TaggedBlock::LayerSectionDivider>(document, header, offset, signature, padding);
		default:
			return std::make_shared<TaggedBlock::Generic>(document, header, offset, signature, taggedBlock.value(), padding);
		}
	}
	else
	{
		PSAPI_LOG_ERROR("TaggedBlock", "Could not find tagged block from key '%s'", keyStr.c_str());
		return NULL;
	}
}


PSAPI_NAMESPACE_END

