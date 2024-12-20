#include "ReferencePointTaggedBlock.h"


PSAPI_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void ReferencePointTaggedBlock::read(File& document, const uint64_t offset, const Signature signature)
{
	m_Key = Enum::TaggedBlockKey::lrReferencePoint;
	m_Offset = offset;
	m_Signature = signature;
	uint32_t length = ReadBinaryData<uint32_t>(document);
	// The data is always two doubles 
	if (length != 16u)
	{
		PSAPI_LOG_ERROR("ReferencePointTaggedBlock", "Invalid size for Reference Point found, expected 16 but got %u", length);
	}
	m_Length = length;
	m_ReferenceX = ReadBinaryData<float64_t>(document);
	m_ReferenceY = ReadBinaryData<float64_t>(document);
	TaggedBlock::totalSize(static_cast<size_t>(length) + 4u + 4u + 4u);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void ReferencePointTaggedBlock::write(File& document, [[maybe_unused]] const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, [[maybe_unused]] const uint16_t padding /* = 1u */)
{
	WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
	WriteBinaryData<uint32_t>(document, Signature("fxrp").m_Value);
	WriteBinaryData<uint32_t>(document, TaggedBlock::totalSize<uint32_t>() - 12u);

	WriteBinaryData<float64_t>(document, m_ReferenceX);
	WriteBinaryData<float64_t>(document, m_ReferenceY);
}


PSAPI_NAMESPACE_END