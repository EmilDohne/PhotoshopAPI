#pragma once

#include "Macros.h"
#include "TaggedBlock.h"

#include "Core/Struct/File.h"
#include "Core/Struct/Signature.h"
#include "Util/ProgressCallback.h"
#include "Util/Enum.h"
#include "Util/Logger.h"

#include <optional>

PSAPI_NAMESPACE_BEGIN


// 16-bit files store this tagged block at the end of the layer and mask information section which contains the 
// layer info section
struct Lr16TaggedBlock : TaggedBlock
{
	LayerInfo m_Data;

	Lr16TaggedBlock() = default;
	Lr16TaggedBlock(LayerInfo& lrInfo) : m_Data(std::move(lrInfo)) {}

	void read(File& document, const FileHeader& header, ProgressCallback& callback, const uint64_t offset, const Signature signature, const uint16_t padding = 1u);
	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u) override;
};

PSAPI_NAMESPACE_END