#include "ColorModeData.h"

#include "FileHeader.h"
#include "Macros.h"
#include "Enum.h"
#include "Core/FileIO/Read.h"
#include "Core/FileIO/Write.h"
#include "Core/Struct/File.h"
#include "Core/Struct/Section.h"
#include "Profiling/Perf/Instrumentor.h"

#include <vector>

#include <cstdint>

PSAPI_NAMESPACE_BEGIN

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
uint64_t ColorModeData::calculateSize(std::shared_ptr<FileHeader> header /*= nullptr*/) const
{
	uint64_t size = 0u;
	size += 4u;	// Size marker
	size += m_Data.size();
	return size;
}


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
void ColorModeData::read(File& document)
{
	PROFILE_FUNCTION();

	m_Offset = 26;
	document.setOffset(m_Offset);

	m_Size = static_cast<uint64_t>(ReadBinaryData<uint32_t>(document)) + 4u;

	// Just dump the data without parsing it
	if (m_Size > 0)
	{
		m_Data = ReadBinaryArray<uint8_t>(document, m_Size);
	}
	else
	{
		m_Data = std::vector<uint8_t>(0);
	}
}


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
void ColorModeData::write(File& document, FileHeader& header)
{
	// This data should be empty for all but 32 bit documents or Indexed color mode sections
	PROFILE_FUNCTION();
	m_Offset = 26;
	
	if (header.m_ColorMode == Enum::ColorMode::Indexed)
	{
		WriteBinaryData<uint32_t>(document, m_Data.size());
		WriteBinaryArray<uint8_t>(document, std::move(m_Data));
		m_Size = m_Data.size() + 4u;
	}
	else if (header.m_Depth == Enum::BitDepth::BD_32)
	{
		// This data is unfortunately undocumented but we just parse the literal values as Photoshop expects these for 32-bit
		// data. These values were taken from a 32-bit file saved in Photoshop 23.3.2 x64. These defaults should also work for 
		// versions up and down but this is as of yet untested
		std::vector<uint8_t> data = {
			0x68, 0x64, 0x72, 0x74, 0x00, 0x00, 0x00, 0x03, 0x3E, 0x6B, 0x85, 0x1F,
			0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x08, 0x00, 0x44, 0x00, 0x65, 0x00, 0x66, 0x00, 0x61,
			0x00, 0x75, 0x00, 0x6C, 0x00, 0x74, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
			0x00, 0xFF, 0x00, 0xFF, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41, 0x80,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x68, 0x64,
			0x72, 0x61, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x41, 0xA0, 0x00, 0x00, 0x41, 0xF0,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};
		// This check is mostly for internal checks as we declare the data here
		if (data.size() != 112u)
		{
			PSAPI_LOG_ERROR("ChannelImageData", "Data size was not 112");
		}
		m_Data = data;
		WriteBinaryData<uint32_t>(document, data.size());
		WriteBinaryArray<uint8_t>(document, std::move(data));
		m_Size = data.size() + 4u;

	}
	else
	{
		if (m_Data.size() > 0)
		{
			PSAPI_LOG_ERROR("ColorModeData", "Invalid size for ColorMode data detected, only indexed colours have data in this \
				section (32-bit files get handled internally)");
		}
		m_Size = 4u;
		WriteBinaryData<uint32_t>(document, 0u);
	}
}

PSAPI_NAMESPACE_END