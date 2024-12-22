#include "UnicodeLayerNameTaggedBlock.h"

#include "Core/FileIO/LengthMarkers.h"

PSAPI_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void UnicodeLayerNameTaggedBlock::read(File& document, const uint64_t offset, const Signature signature, const uint16_t padding /*= 1u*/)
{
	m_Key = Enum::TaggedBlockKey::lrUnicodeName;
	m_Offset = offset;
	m_Signature = signature;
	uint32_t length = ReadBinaryData<uint32_t>(document);
	length = RoundUpToMultiple<uint32_t>(length, padding);
	m_Length = length;

	m_Name.read(document, 4u);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void UnicodeLayerNameTaggedBlock::write(File& document, [[maybe_unused]] const FileHeader& header, [[maybe_unused]] ProgressCallback& callback, const uint16_t padding /*= 1u*/)
{
	WriteBinaryData<uint32_t>(document, Signature("8BIM").m_Value);
	WriteBinaryData<uint32_t>(document, Signature("luni").m_Value);
	Impl::ScopedLengthBlock<uint32_t> len_block(document, padding);

	m_Name.write(document);
}


PSAPI_NAMESPACE_END