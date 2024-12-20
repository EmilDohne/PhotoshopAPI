#pragma once

#include "Macros.h"
#include "TaggedBlock.h"

#include "Core/Struct/File.h"
#include "Core/Struct/Signature.h"
#include "Util/ProgressCallback.h"
#include "Util/Enum.h"
#include "Util/StringUtil.h"
#include "Util/Logger.h"

#include <optional>

PSAPI_NAMESPACE_BEGIN

// This tagged block demarkates the start or end of a layer section (group). It may additionally store the Passthrough blend mode
struct LrSectionTaggedBlock : TaggedBlock
{
	Enum::SectionDivider m_Type = Enum::SectionDivider::Any;

	// This is a bit weird, but if the blend mode for the layer is Passthrough, it stores BlendMode::Normal
	// on the layer itself and includes the blend mode over here. This is only present if the length is >= 12u
	std::optional<Enum::BlendMode> m_BlendMode;

	LrSectionTaggedBlock() = default;
	LrSectionTaggedBlock(Enum::SectionDivider sectionDivider, std::optional<Enum::BlendMode> blendMode) :
		m_Type(sectionDivider),
		m_BlendMode(blendMode)
	{
		m_Key = Enum::TaggedBlockKey::lrSectionDivider;
		TaggedBlock::totalSize(4u);		// Signature
		TaggedBlock::addTotalSize(4u);	// Key
		TaggedBlock::addTotalSize(4u);	// Length marker
		TaggedBlock::addTotalSize(4u);	// LrSection type
		if (blendMode.has_value())
		{
			TaggedBlock::addTotalSize(4u);	// LrSection Signature
			TaggedBlock::addTotalSize(4u);	// LrSection Blendmode Key
		}
	};

	void read(File& document, const uint64_t offset, const Signature signature, const uint16_t padding = 1u);
	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u) override;
};

PSAPI_NAMESPACE_END