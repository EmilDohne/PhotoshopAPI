#pragma once

#include "Macros.h"
#include "TaggedBlock.h"

#include "Core/Struct/File.h"
#include "Core/Struct/UnicodeString.h"
#include "Core/Struct/Signature.h"
#include "Util/ProgressCallback.h"
#include "Util/Enum.h"
#include "Util/Logger.h"

PSAPI_NAMESPACE_BEGIN

/// The layer name stored in UTF16 BE order on disk, this is the preferred way of retrieving the layer name as it is 
/// not limited to 255 chars like the layers PascalString but instead can hold up to 2^32 code points or 2^33 bytes
/// This tagged block is not required but Photoshop usually will always write this out in more modern versions 
struct UnicodeLayerNameTaggedBlock : TaggedBlock
{
	UnicodeString m_Name = {};

	UnicodeLayerNameTaggedBlock() = default;
	UnicodeLayerNameTaggedBlock(std::string name, const uint8_t padding = 1u)
	{
		m_Name = UnicodeString(name, padding);
	}

	void read(File& document, const uint64_t offset, const Signature signature, const uint16_t padding = 1u);
	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u) override;
};

PSAPI_NAMESPACE_END