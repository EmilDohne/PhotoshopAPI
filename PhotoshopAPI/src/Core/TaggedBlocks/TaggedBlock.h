#pragma once

#include "Macros.h"
#include "Util/Enum.h"
#include "PhotoshopFile/FileHeader.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "Core/Struct/File.h"
#include "Core/Struct/Signature.h"
#include "Util/Logger.h"

#include "Core/FileIO/Write.h"
#include "Core/FileIO/Util.h"

#include <memory>
#include <variant>

PSAPI_NAMESPACE_BEGIN


// Base TaggedBlock, this is the default implementation for any non-specialized tagged blocks and will read
// and write the raw byte data but not attempt to decode this. Acts as a simple pass through.
struct TaggedBlock
{
	Signature m_Signature = Signature("8BIM");
	uint64_t m_Offset = 0u;	// Demarkates the start of the taggedblock, not the start of the data
	std::variant<uint32_t, uint64_t> m_Length = uint32_t{ 0u };
	std::vector<std::byte> m_Data;

	Enum::TaggedBlockKey getKey() const noexcept{ return m_Key; };

	virtual ~TaggedBlock() = default;
	TaggedBlock() = default;

	// Read a TaggedBlock from a file
	void read(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const Enum::TaggedBlockKey key, const uint16_t padding = 1u);
	virtual void write(File& document, [[maybe_unused]] const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, [[maybe_unused]] const uint16_t padding = 1u);
protected:
	Enum::TaggedBlockKey m_Key = Enum::TaggedBlockKey::Unknown;
};


PSAPI_NAMESPACE_END