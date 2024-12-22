#include "TaggedBlock.h"

#include "Macros.h"
#include "Enum.h"
#include "Logger.h"
#include "StringUtil.h"
#include "Core/Struct/Signature.h"
#include "Core/Struct/File.h"
#include "PhotoshopFile/FileHeader.h"
#include "Core/FileIO/Read.h"
#include "Core/FileIO/Write.h"

#include <cassert>
#include <ctime>

PSAPI_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void TaggedBlock::read(File& document, const FileHeader& header, const uint64_t offset, const Signature signature, const Enum::TaggedBlockKey key, const uint16_t padding /* = 1u */)
{
	m_Offset = offset;
	m_Signature = signature;
	m_Key = key;
	if (Enum::isTaggedBlockSizeUint64(m_Key) && header.m_Version == Enum::Version::Psb)
	{
		uint64_t length = ReadBinaryData<uint64_t>(document);
		length = RoundUpToMultiple<uint64_t>(length, padding);
		m_Length = length;
		document.skip(length);
	}
	else
	{
		uint32_t length = ReadBinaryData<uint32_t>(document);
		length = RoundUpToMultiple<uint32_t>(length, padding);
		m_Length = length;
		document.skip(length);
	}
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void TaggedBlock::write(File& document, [[maybe_unused]] const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, [[maybe_unused]] const uint16_t padding /* = 1u */)
{

	// Signatures are specified as being either '8BIM' or '8B64'. However, it isnt specified when we use which one.
	// For simplicity we will just write '8BIM' all the time and only write other signatures if we encounter them.
	// The 'FMsk' and 'cinf' tagged blocks for example have '8B64' in PSB mode
	WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
	std::optional<std::vector<std::string>> keyStr = Enum::getTaggedBlockKey<Enum::TaggedBlockKey, std::vector<std::string>>(m_Key);
	if (!keyStr.has_value())
	{
		PSAPI_LOG_ERROR("TaggedBlock", "Was unable to extract a string from the tagged block key");
	}
	else
	{
		// We use the first found value from the key matches
		WriteBinaryData<uint32_t>(document, Signature(keyStr.value()[0]).m_Value);
	}

	if (isTaggedBlockSizeUint64(m_Key) && header.m_Version == Enum::Version::Psb)
	{
		WriteBinaryData<uint64_t>(document, 0u);
	}
	else
	{
		WriteBinaryData<uint32_t>(document, 0u);
	}

	// No need to write any padding bytes here as the section will already be aligned to all the possible padding sizes (1u for LayerRecord TaggedBlocks and 4u
	// for "Global" Tagged Blocks (found at the end of the LayerAndMaskInformation section))
}

PSAPI_NAMESPACE_END