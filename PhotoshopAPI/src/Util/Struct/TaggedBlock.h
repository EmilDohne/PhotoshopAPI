#pragma once

#include "Macros.h"
#include "Enum.h"
#include "PhotoshopFile/FileHeader.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "Struct/File.h"
#include "Struct/Signature.h"

#include <memory>
#include <variant>

PSAPI_NAMESPACE_BEGIN



// Generic Tagged Block which does not hold any data. If you wish to parse further tagged blocks extend this struct and add an implementation
struct TaggedBlock
{
	Signature m_Signature;
	uint64_t m_Offset = 0u;	// Demarkates the start of the taggedblock, not the start of the data
	std::variant<uint32_t, uint64_t> m_Length;

	uint64_t getTotalSize() const { return m_TotalLength; };
	Enum::TaggedBlockKey getKey() const { return m_Key; };

	virtual ~TaggedBlock() = default;
	TaggedBlock() = default;
	TaggedBlock(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const Enum::TaggedBlockKey key, const uint16_t padding = 1u);
protected:
	Enum::TaggedBlockKey m_Key = Enum::TaggedBlockKey::Unknown;
	// The length of the tagged block with all the the signature, key and length marker
	// use this value to determine how long the total structure is
	uint64_t m_TotalLength = 0u;
};


// This tagged block demarkates the start or end of a layer section (group). It may additionally store the Passthrough blend mode
struct LrSectionTaggedBlock : TaggedBlock
{
	Enum::SectionDivider m_Type = Enum::SectionDivider::Any;

	// This is a bit weird, but if the blend mode for the layer is Passthrough, it stores BlendMode::Normal
	// on the layer itself and includes the blend mode over here. This is only present if the length is >= 12u
	std::optional<Enum::BlendMode> m_BlendMode;

	LrSectionTaggedBlock(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const uint16_t padding = 1u);
};


// 16-bit files store this tagged block at the end of the layer and mask information section which contains the 
// layer info section
struct Lr16TaggedBlock : TaggedBlock
{
	LayerInfo m_Data;

	Lr16TaggedBlock(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const uint16_t padding = 1u);
};



// 32-bit files store this tagged block at the end of the layer and mask information section which contains the 
// layer info section
struct Lr32TaggedBlock : TaggedBlock
{
	LayerInfo m_Data;

	Lr32TaggedBlock(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const uint16_t padding = 1u);
};


PSAPI_NAMESPACE_END