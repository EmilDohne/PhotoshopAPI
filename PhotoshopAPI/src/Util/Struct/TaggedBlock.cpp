#include "TaggedBlock.h"

#include "Signature.h"
#include "File.h"
#include "../Enum.h"
#include "../Logger.h"
#include "../StringUtil.h"
#include "../../Macros.h"
#include "../../PhotoshopFile/LayerAndMaskInformation.h"

PSAPI_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TaggedBlock::Generic_32::Generic_32(File& document, const uint64_t offset, const Signature signature, const Enum::TaggedBlockKey key)
{
	this->m_Offset = offset;
	this->m_Signature = signature;
	this->m_Key = key;
	this->m_Length = ReadBinaryData<uint32_t>(document);
	this->m_Length = RoundUpToMultiple<uint32_t>(this->m_Length, 2u);
	this->m_Data = ReadBinaryArray<uint8_t>(document, this->m_Length);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TaggedBlock::Generic_64::Generic_64(File& document, const uint64_t offset, const Signature signature, const Enum::TaggedBlockKey key)
{
	this->m_Offset = offset;
	this->m_Signature = signature;
	this->m_Key = key;
	this->m_Length = ReadBinaryData<uint64_t>(document);
	this->m_Length = RoundUpToMultiple<uint64_t>(this->m_Length, 2u);
	this->m_Data = ReadBinaryArray<uint8_t>(document, this->m_Length);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TaggedBlock::Lr16::Lr16(File& document, const FileHeader& header, const uint64_t offset, const Signature signature)
{
	this->m_Offset = offset;
	this->m_Signature = signature;
	this->m_Length = ReadBinaryData<uint64_t>(document);
	this->m_Length = RoundUpToMultiple<uint64_t>(this->m_Length, 2u);
	this->m_Data = LayerInfo(document, header, document.getOffset());
};


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TaggedBlock::Lr32::Lr32(File& document, const FileHeader& header, const uint64_t offset, const Signature signature)
{
	this->m_Offset = offset;
	this->m_Signature = signature;
	this->m_Length = ReadBinaryData<uint64_t>(document);
	this->m_Length = RoundUpToMultiple<uint64_t>(this->m_Length, 2u);
	this->m_Data = LayerInfo(document, header, document.getOffset());
};


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::unique_ptr<TaggedBlock::Base> readTaggedBlock(File& document, const FileHeader& header)
{
	const uint64_t offset = document.getOffset();
	Signature signature = Signature(ReadBinaryData<uint32_t>(document));
	if (signature != Signature("8BIM") && signature != Signature("8B64"))
	{
		PSAPI_LOG_ERROR("LayerRecord", "Signature does not match '8BIM' or '8B64', got '%s' instead",
			uint32ToString(signature.m_Value))
	}
	std::string keyStr = uint32ToString(ReadBinaryData<uint32_t>(document));
	std::optional<Enum::TaggedBlockKey> taggedBlock = Enum::getTaggedBlockKey<std::string, Enum::TaggedBlockKey>(keyStr);
	
	if (taggedBlock.has_value())
	{
		switch (taggedBlock.value())
		{
		case Enum::TaggedBlockKey::Lr16:
			return std::make_unique<TaggedBlock::Lr16>(document, header, offset, signature);
		case Enum::TaggedBlockKey::Lr32:
			return std::make_unique<TaggedBlock::Lr32>(document, header, offset, signature);
		case Enum::TaggedBlockKey::Layr:
			return std::make_unique<TaggedBlock::Layr>(document, header, offset, signature);
		case Enum::TaggedBlockKey::Alph:
			return std::make_unique<TaggedBlock::Alpha>(document, header, offset, signature);
		case Enum::TaggedBlockKey::lrSavingMergedTransparency:
			return std::make_unique<TaggedBlock::LayerSavingMergedTransparency>(document, header, offset, signature);
		case Enum::TaggedBlockKey::lrFilterMask:
			return std::make_unique<TaggedBlock::LayerFilterMask>(document, header, offset, signature);
		case Enum::TaggedBlockKey::lrFilterEffects:
			return std::make_unique<TaggedBlock::LayerFilterEffects>(document, header, offset, signature);
		case Enum::TaggedBlockKey::lrLinked_8Byte:
			return std::make_unique<TaggedBlock::LinkedLayer_8Byte>(document, header, offset, signature);
		case Enum::TaggedBlockKey::lrPixelSourceData:
			return std::make_unique<TaggedBlock::PixelSourceData>(document, header, offset, signature);
		case Enum::TaggedBlockKey::lrCompositorUsed:
			return std::make_unique<TaggedBlock::CompositorUsed>(document, header, offset, signature);
		default:
			return std::make_unique<TaggedBlock::Generic_32>(document, offset, signature, taggedBlock);
		}
	}
	else
	{
		PSAPI_LOG_ERROR("TaggedBlock", "Could not find tagged block from key '%s'", keyStr.c_str());
	}
}


PSAPI_NAMESPACE_END

