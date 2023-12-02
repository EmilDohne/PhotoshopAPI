#include "ResourceBlock.h"

#include "Read.h"
#include "Enum.h"
#include "Logger.h"

PSAPI_NAMESPACE_BEGIN

// Sequential Read of a single Image Resource Block, in its current state it just dumps the 
// data rather than parsing it.
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
ResourceBlock::ResourceBlock(File& document)
{
	Signature signature = Signature(ReadBinaryData<uint32_t>(document));
	if (signature != Signature("8BIM"))
	{
		PSAPI_LOG_ERROR("ResourceBlock", "Signature does not match '8BIM', got '%c%c%c%c' instead",
			signature.m_Representation[0],
			signature.m_Representation[1],
			signature.m_Representation[2],
			signature.m_Representation[3])
	}
	m_UniqueId = Enum::intToImageResource(ReadBinaryData<uint16_t>(document));
	m_Name = PascalString(document, 2u);
	m_Size = RoundUpToMultiple(ReadBinaryData<uint32_t>(document), 2u);
	m_Data = ReadBinaryArray<uint8_t>(document, m_Size);

	m_BlockSize = 4u + 2u + m_Name.m_Size + 4u + m_Size;
}


PSAPI_NAMESPACE_END