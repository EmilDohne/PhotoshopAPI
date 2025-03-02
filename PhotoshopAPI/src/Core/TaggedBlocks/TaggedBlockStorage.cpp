#include "TaggedBlockStorage.h"

#include "Macros.h"
#include "Core/FileIO/Read.h"
#include "Core/FileIO/Write.h"
#include "Logger.h"
#include "StringUtil.h"


#include "TaggedBlock.h"
#include "LinkedLayerTaggedBlock.h"
#include "LrSectionTaggedBlock.h"
#include "Lr16TaggedBlock.h"
#include "Lr32TaggedBlock.h"
#include "PlacedLayerTaggedBlock.h"
#include "ProtectedSettingTaggedBlock.h"
#include "ReferencePointTaggedBlock.h"
#include "UnicodeLayerNameTaggedBlock.h"
#include "TypeToolTaggedBlock.h"

PSAPI_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
TaggedBlockStorage::TaggedBlockStorage(std::vector<std::shared_ptr<TaggedBlock>> taggedBlocks)
{
	m_TaggedBlocks = std::move(taggedBlocks);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
std::shared_ptr<T> TaggedBlockStorage::getTaggedBlockView(const Enum::TaggedBlockKey key) const
{
	// iterate all tagged blocks and compare keys as well as trying to cast the pointer
	for (auto& taggedBlock : m_TaggedBlocks)
	{
		if (taggedBlock->getKey() == key)
		{
			if (std::shared_ptr<T> downcastedPtr = std::dynamic_pointer_cast<T>(taggedBlock))
			{
				return downcastedPtr;
			}
		}
	}
	return nullptr;
}


// Instantiate the templates for all the tagged block types. Unfortunately required to avoid circular
// dependencies
template std::shared_ptr<TaggedBlock>					TaggedBlockStorage::getTaggedBlockView(const Enum::TaggedBlockKey key) const;
template std::shared_ptr<Lr16TaggedBlock>				TaggedBlockStorage::getTaggedBlockView(const Enum::TaggedBlockKey key) const;
template std::shared_ptr<Lr32TaggedBlock>				TaggedBlockStorage::getTaggedBlockView(const Enum::TaggedBlockKey key) const;
template std::shared_ptr<LrSectionTaggedBlock>			TaggedBlockStorage::getTaggedBlockView(const Enum::TaggedBlockKey key) const;
template std::shared_ptr<ReferencePointTaggedBlock>		TaggedBlockStorage::getTaggedBlockView(const Enum::TaggedBlockKey key) const;
template std::shared_ptr<UnicodeLayerNameTaggedBlock>	TaggedBlockStorage::getTaggedBlockView(const Enum::TaggedBlockKey key) const;
template std::shared_ptr<ProtectedSettingTaggedBlock>	TaggedBlockStorage::getTaggedBlockView(const Enum::TaggedBlockKey key) const;
template std::shared_ptr<PlacedLayerTaggedBlock>		TaggedBlockStorage::getTaggedBlockView(const Enum::TaggedBlockKey key) const;
template std::shared_ptr<PlacedLayerDataTaggedBlock>	TaggedBlockStorage::getTaggedBlockView(const Enum::TaggedBlockKey key) const;
template std::shared_ptr<LinkedLayerTaggedBlock>		TaggedBlockStorage::getTaggedBlockView(const Enum::TaggedBlockKey key) const;
template std::shared_ptr<TypeToolTaggedBlock>			TaggedBlockStorage::getTaggedBlockView(const Enum::TaggedBlockKey key) const;


template <typename T>
std::shared_ptr<T> TaggedBlockStorage::getTaggedBlockView() const
{
	for (auto& taggedBlock : m_TaggedBlocks)
	{
		if (std::shared_ptr<T> downcastedPtr = std::dynamic_pointer_cast<T>(taggedBlock))
		{
			return downcastedPtr;
		}
	}
	return nullptr;
}

// Instantiate the templates for all the tagged block types. Unfortunately required to avoid circular
// dependencies
template std::shared_ptr<TaggedBlock>					TaggedBlockStorage::getTaggedBlockView() const;
template std::shared_ptr<Lr16TaggedBlock>				TaggedBlockStorage::getTaggedBlockView() const;
template std::shared_ptr<Lr32TaggedBlock>				TaggedBlockStorage::getTaggedBlockView() const;
template std::shared_ptr<LrSectionTaggedBlock>			TaggedBlockStorage::getTaggedBlockView() const;
template std::shared_ptr<ReferencePointTaggedBlock>		TaggedBlockStorage::getTaggedBlockView() const;
template std::shared_ptr<UnicodeLayerNameTaggedBlock>	TaggedBlockStorage::getTaggedBlockView() const;
template std::shared_ptr<ProtectedSettingTaggedBlock>	TaggedBlockStorage::getTaggedBlockView() const;
template std::shared_ptr<PlacedLayerTaggedBlock>		TaggedBlockStorage::getTaggedBlockView() const;
template std::shared_ptr<PlacedLayerDataTaggedBlock>	TaggedBlockStorage::getTaggedBlockView() const;
template std::shared_ptr<LinkedLayerTaggedBlock>		TaggedBlockStorage::getTaggedBlockView() const;
template std::shared_ptr<TypeToolTaggedBlock>			TaggedBlockStorage::getTaggedBlockView() const;


template <typename T>
std::vector<std::shared_ptr<T>> TaggedBlockStorage::get_tagged_blocks() const
{
	std::vector<std::shared_ptr<T>> blocks;
	for (auto& taggedBlock : m_TaggedBlocks)
	{
		if (std::shared_ptr<T> downcastedPtr = std::dynamic_pointer_cast<T>(taggedBlock))
		{
			blocks.push_back(downcastedPtr);
		}
	}
	return blocks;
}


// Instantiate the templates for all the tagged block types. Unfortunately required to avoid circular
// dependencies
template std::vector<std::shared_ptr<TaggedBlock>>					TaggedBlockStorage::get_tagged_blocks() const;
template std::vector<std::shared_ptr<Lr16TaggedBlock>>				TaggedBlockStorage::get_tagged_blocks() const;
template std::vector<std::shared_ptr<Lr32TaggedBlock>>				TaggedBlockStorage::get_tagged_blocks() const;
template std::vector<std::shared_ptr<LrSectionTaggedBlock>>			TaggedBlockStorage::get_tagged_blocks() const;
template std::vector<std::shared_ptr<ReferencePointTaggedBlock>>	TaggedBlockStorage::get_tagged_blocks() const;
template std::vector<std::shared_ptr<UnicodeLayerNameTaggedBlock>>	TaggedBlockStorage::get_tagged_blocks() const;
template std::vector<std::shared_ptr<ProtectedSettingTaggedBlock>>	TaggedBlockStorage::get_tagged_blocks() const;
template std::vector<std::shared_ptr<PlacedLayerTaggedBlock>>		TaggedBlockStorage::get_tagged_blocks() const;
template std::vector<std::shared_ptr<PlacedLayerDataTaggedBlock>>	TaggedBlockStorage::get_tagged_blocks() const;
template std::vector<std::shared_ptr<LinkedLayerTaggedBlock>>		TaggedBlockStorage::get_tagged_blocks() const;
template std::vector<std::shared_ptr<TypeToolTaggedBlock>>			TaggedBlockStorage::get_tagged_blocks() const;



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
const std::shared_ptr<TaggedBlock> TaggedBlockStorage::readTaggedBlock(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding)
{
	const uint64_t offset = document.getOffset();
	Signature signature = Signature(ReadBinaryData<uint32_t>(document));
	if (signature != Signature("8BIM") && signature != Signature("8B64"))
	{
		// This is to make sure we don't accidentally null terminate and upset the string gods.
		auto getPrintableChar = [](char c) { return c ? c : ' '; };
		PSAPI_LOG_ERROR("TaggedBlock", "Signature does not match '8BIM' or '8B64', got '%c%c%c%c' instead",
			getPrintableChar(signature.m_Representation[0]),
			getPrintableChar(signature.m_Representation[1]),
			getPrintableChar(signature.m_Representation[2]),
			getPrintableChar(signature.m_Representation[3]));
	}
	std::string keyStr = uint32ToString(ReadBinaryData<uint32_t>(document));
	std::optional<Enum::TaggedBlockKey> taggedBlock = Enum::getTaggedBlockKey<std::string, Enum::TaggedBlockKey>(keyStr);

	if (taggedBlock.has_value())
	{
		if (taggedBlock.value() == Enum::TaggedBlockKey::Lr16)
		{
			auto lr16TaggedBlock = std::make_shared<Lr16TaggedBlock>();
			lr16TaggedBlock->read(document, header, callback, offset, signature, padding);
			this->m_TaggedBlocks.push_back(lr16TaggedBlock);
			return lr16TaggedBlock;
		}
		else if (taggedBlock.value() == Enum::TaggedBlockKey::Lr32)
		{
			auto lr32TaggedBlock = std::make_shared<Lr32TaggedBlock>();
			lr32TaggedBlock->read(document, header, callback, offset, signature, padding);
			this->m_TaggedBlocks.push_back(lr32TaggedBlock);
			return lr32TaggedBlock;
		}
		else if (taggedBlock.value() == Enum::TaggedBlockKey::lrSectionDivider)
		{
			auto lrSectionTaggedBlock = std::make_shared<LrSectionTaggedBlock>();
			lrSectionTaggedBlock->read(document, offset, signature, padding);
			this->m_TaggedBlocks.push_back(lrSectionTaggedBlock);
			return lrSectionTaggedBlock;
		}
		else if (taggedBlock.value() == Enum::TaggedBlockKey::lrReferencePoint)
		{
			auto lrReferencePointBlock = std::make_shared<ReferencePointTaggedBlock>();
			lrReferencePointBlock->read(document, offset, signature);
			this->m_TaggedBlocks.push_back(lrReferencePointBlock);
			return lrReferencePointBlock;
		}
		else if (taggedBlock.value() == Enum::TaggedBlockKey::lrUnicodeName)
		{
			auto lrUnicodeNameBlock = std::make_shared<UnicodeLayerNameTaggedBlock>();
			lrUnicodeNameBlock->read(document, offset, signature, padding);
			this->m_TaggedBlocks.push_back(lrUnicodeNameBlock);
			return lrUnicodeNameBlock;
		}
		else if (taggedBlock.value() == Enum::TaggedBlockKey::lrProtectedSetting)
		{
			auto lrProtectionTaggedBlock = std::make_shared<ProtectedSettingTaggedBlock>();
			lrProtectionTaggedBlock->read(document, offset, signature);
			this->m_TaggedBlocks.push_back(lrProtectionTaggedBlock);
			return lrProtectionTaggedBlock;
		}
		else if (taggedBlock.value() == Enum::TaggedBlockKey::lrPlaced)
		{
			auto lrPlacedBlock = std::make_shared<PlacedLayerTaggedBlock>();
			lrPlacedBlock->read(document, offset, taggedBlock.value(), signature);
			this->m_TaggedBlocks.push_back(lrPlacedBlock);
			return lrPlacedBlock;
		}
		else if (taggedBlock.value() == Enum::TaggedBlockKey::lrPlacedData)
		{
			auto lrPlacedDataBlock = std::make_shared<PlacedLayerDataTaggedBlock>();
			lrPlacedDataBlock->read(document, offset, taggedBlock.value(), signature);
			this->m_TaggedBlocks.push_back(lrPlacedDataBlock);
			return lrPlacedDataBlock;
		}
		else if (taggedBlock.value() == Enum::TaggedBlockKey::lrLinked || taggedBlock.value() == Enum::TaggedBlockKey::lrLinked_8Byte)
		{
			auto lrLinkedTaggedBlock = std::make_shared<LinkedLayerTaggedBlock>();
			lrLinkedTaggedBlock->read(document, header, offset, taggedBlock.value(), signature, padding);
			this->m_TaggedBlocks.push_back(lrLinkedTaggedBlock);
			return lrLinkedTaggedBlock;
		}
		else if (taggedBlock.value() == Enum::TaggedBlockKey::lrTypeTool)
		{
			auto type_tagged_block = std::make_shared<TypeToolTaggedBlock>();
			type_tagged_block->read(document, offset, signature, padding);
			this->m_TaggedBlocks.push_back(type_tagged_block);
			return type_tagged_block;
		}
		else
		{
			auto baseTaggedBlock = std::make_shared<TaggedBlock>();
			baseTaggedBlock->read(document, header, offset, signature, taggedBlock.value(), padding);
			this->m_TaggedBlocks.push_back(baseTaggedBlock);
			return baseTaggedBlock;
		}
	}
	else
	{
		PSAPI_LOG_ERROR("TaggedBlock", "Could not find tagged block from key '%s'", keyStr.c_str());
		return nullptr;
	}
}

void TaggedBlockStorage::write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding) const
{
	for (const auto& block : m_TaggedBlocks)
	{
		block->write(document, header, callback, padding);
	}
	// Since the tagged blocks themselves are aligned to padding we dont need to pad the rest of this section manually
}

PSAPI_NAMESPACE_END