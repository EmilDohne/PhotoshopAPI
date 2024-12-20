#pragma once

#include "Macros.h"
#include "TaggedBlock.h"

#include "Core/Struct/File.h"
#include "Core/Struct/Signature.h"
#include "Util/ProgressCallback.h"
#include "Util/Enum.h"
#include "Util/Logger.h"

PSAPI_NAMESPACE_BEGIN


/// The layers' pixel protection settings. This is internally a uint32_t of which only the first byte seems to hold relevant 
/// information with the rest being for padding and/or aligment. 
struct ProtectedSettingTaggedBlock : TaggedBlock
{
	bool m_IsLocked = false;	// 01000000 of the first byte

	ProtectedSettingTaggedBlock() = default;
	ProtectedSettingTaggedBlock(bool isLocked) : m_IsLocked(isLocked)
	{
		// This is the size of a uin32_t + 4 bytes for the signature, 4 bytes for the key and 4 bytes for the length
		TaggedBlock::totalSize(4u + 4u + 4u + 4u);
	};

	void read(File& document, const uint64_t offset, const Signature signature);
	void write(File& document, const FileHeader& header, ProgressCallback& callback, const uint16_t padding = 1u) override;
};


PSAPI_NAMESPACE_END