#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Struct/File.h"
#include "Struct/Section.h"
#include "FileIO/Write.h"

#include "blosc2.h"


PSAPI_NAMESPACE_BEGIN


struct ImageData : public FileSection
{
	
	inline uint64_t calculateSize(std::shared_ptr<FileHeader> header /* = nullptr */) const override { return 0; };

	// Write out an empty image data section from the number of channels. This data is uncompressed
	inline void write(File& document, const FileHeader& header)
	{
		// Compression marker
		WriteBinaryData<uint16_t>(document, 0u);
		// Write out empty data for all of the channels
		if (header.m_Depth == Enum::BitDepth::BD_8)
		{
			std::vector<uint8_t> emptyData(static_cast<uint64_t>(header.m_Width) * header.m_Height, 0u);
			for (int i = 0; i < header.m_NumChannels; ++i)
			{
				WriteBinaryArray<uint8_t>(document, emptyData);
			}
		}
		else if (header.m_Depth == Enum::BitDepth::BD_16)
		{
			std::vector<uint16_t> emptyData(static_cast<uint64_t>(header.m_Width) * header.m_Height, 0u);
			for (int i = 0; i < header.m_NumChannels; ++i)
			{
				WriteBinaryArray<uint16_t>(document, emptyData);
			}
		}
		else if (header.m_Depth == Enum::BitDepth::BD_32)
		{
			std::vector<float32_t> emptyData(static_cast<uint64_t>(header.m_Width) * header.m_Height, 0u);
			for (int i = 0; i < header.m_NumChannels; ++i)
			{
				WriteBinaryArray<float32_t>(document, emptyData);
			}
		}
	}
};


PSAPI_NAMESPACE_END