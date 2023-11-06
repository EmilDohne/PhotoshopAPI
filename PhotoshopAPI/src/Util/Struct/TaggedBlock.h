#pragma once

#include "../../Macros.h"
#include "../../PhotoshopFile/FileHeader.h"
#include "../Enum.h"
#include "File.h"
#include "Signature.h"

#include <memory>
#include <variant>

PSAPI_NAMESPACE_BEGIN


namespace TaggedBlock
{
	struct Base
	{
		Signature m_Signature;
		uint64_t m_Offset;
		std::variant<uint32_t, uint64_t> m_Length;

		uint64_t getTotalSize() { return this->m_TotalLength; };
	protected:
		Enum::TaggedBlockKey m_Key;
		// The length of the tagged block with all the the signature, key and length marker
		// use this value to determine how long the total structure is
		uint64_t m_TotalLength;
	};

	struct Generic : Base
	{
		std::vector<uint8_t> m_Data;

		Generic(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const Enum::TaggedBlockKey key);
	};
}

// Return a pointer to a Tagged block based on the key it reads
std::unique_ptr<TaggedBlock::Base> readTaggedBlock(File& document, const FileHeader& header);

PSAPI_NAMESPACE_END