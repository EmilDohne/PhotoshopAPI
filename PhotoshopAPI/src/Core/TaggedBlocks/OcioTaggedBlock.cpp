#include "OcioTaggedBlock.h"

#include "Core/FileIO/LengthMarkers.h"

PSAPI_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void OcioTaggedBlock::read(File& document, const uint64_t offset, const Enum::TaggedBlockKey key, const Signature signature, const uint16_t padding)
{
	m_Key = key;
	m_Offset = offset;
	m_Signature = signature;

	m_Length = RoundUpToMultiple<uint32_t>(ReadBinaryData<uint32_t>(document), padding);
	auto len_offset = document.get_offset();

	auto descriptorVersion = ReadBinaryData<uint32_t>(document);
	if (descriptorVersion != 16)
	{
		PSAPI_LOG_ERROR("OCIO", "Unknown descriptor version encountered. Descriptor Version: %d. Expected 16 for this", descriptorVersion);
	}

	m_Descriptor = std::make_unique<Descriptors::Descriptor>();
	m_Descriptor->read(document);
	// Manually skip to the end as this section may be padded
	document.setOffset(len_offset + std::get<uint32_t>(m_Length));
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void OcioTaggedBlock::write(File& document, [[maybe_unused]] const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, [[maybe_unused]] const uint16_t padding /*= 1u*/)
{
	WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
	WriteBinaryData<uint32_t>(document, Signature("OCIO").m_Value);

	Impl::ScopedLengthBlock<uint32_t> len_block(document, padding);
	WriteBinaryData<uint32_t>(document, 16u);
	m_Descriptor->write(document);
}

PSAPI_NAMESPACE_END