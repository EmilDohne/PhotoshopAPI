#pragma once

#include "Macros.h"
#include "Enum.h"
#include "PhotoshopFile/FileHeader.h"
#include "Struct/File.h"
#include "Struct/Signature.h"

#include <memory>
#include <variant>

PSAPI_NAMESPACE_BEGIN


namespace TaggedBlock
{
	struct Base
	{
		Signature m_Signature;
		uint64_t m_Offset = 0u;	// Demarkates the start of the taggedblock, not the start of the data
		std::variant<uint32_t, uint64_t> m_Length;

		uint64_t getTotalSize() { return m_TotalLength; };
	protected:
		Enum::TaggedBlockKey m_Key = Enum::TaggedBlockKey::Unknown;
		// The length of the tagged block with all the the signature, key and length marker
		// use this value to determine how long the total structure is
		uint64_t m_TotalLength = 0u;
	};

	struct Generic : Base
	{
		std::vector<uint8_t> m_Data;

		Generic(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const Enum::TaggedBlockKey key, const uint16_t padding = 1u);
	};
}

// Return a pointer to a Tagged block based on the key it reads
std::unique_ptr<TaggedBlock::Base> readTaggedBlock(File& document, const FileHeader& header, const uint16_t padding = 1u);

PSAPI_NAMESPACE_END