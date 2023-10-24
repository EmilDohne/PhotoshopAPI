#include "ResourceBlock.h"

#include "../Read.h"
#include "../Enum.h"
#include "../Logger.h"

PSAPI_NAMESPACE_BEGIN

// Sequential Read of a single Image Resource Block, in its current state it just dumps the 
// data rather than parsing it.
// ----------------------------------------------------------------------------------------
ResourceBlock::ResourceBlock(File& document)
{
	this->m_Signature = Signature(ReadBinaryData<uint32_t>(document));
	if (this->m_Signature != Signature("8BIM"))
	{
		PSAPI_LOG_ERROR("ResourceBlock", "Signature does not match '8BIM', got %c%c%c%c instead",
			this->m_Signature.m_Representation[0],
			this->m_Signature.m_Representation[1],
			this->m_Signature.m_Representation[2],
			this->m_Signature.m_Representation[3])
	}
	this->m_UniqueId = Enum::intToImageResource(ReadBinaryData<uint16_t>(document));
	this->m_Name = PascalString(document, 2u);
	this->m_Size = RoundUpToMultiple(ReadBinaryData<uint32_t>(document), 2u);
	this->m_Data = ReadBinaryArray<char>(document, this->m_Size);

	this->m_BlockSize = 4u + 2u + this->m_Name.m_Size + 4u + this->m_Size;
}


PSAPI_NAMESPACE_END