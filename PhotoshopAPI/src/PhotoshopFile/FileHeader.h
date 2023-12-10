#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Struct/File.h"
#include "Struct/Signature.h"
#include "Struct/Section.h"

#include <cstdint>


PSAPI_NAMESPACE_BEGIN


struct FileHeader : public FileSection
{
	Signature m_Signature;			// Has to be 8BPS
	Enum::Version m_Version;		// 1 or 2 for PSD and PSB respectively
	uint16_t m_NumChannels;			// Supported range by Photoshop is 1-56
	uint32_t m_Height;				// 1 - 30,000 for PSD and 1 - 300,000 for PSB
	uint32_t m_Width;				// 1 - 30,000 for PSD and 1 - 300,000 for PSB
	Enum::BitDepth m_Depth;			// Depth of the Document
	Enum::ColorMode m_ColorMode;	// Color Mode of the File

	FileHeader() = default;
	// Note that we do not initialize any variables for FileSection here as that will be handled once we write the file
	FileHeader(Signature signature, Enum::Version version, uint16_t numChannels, uint32_t width, uint32_t height, Enum::BitDepth depth, Enum::ColorMode colorMode) :
		m_Signature(signature),
		m_Version(version),
		m_NumChannels(numChannels),
		m_Height(height),
		m_Width(width),
		m_Depth(depth),
		m_ColorMode(colorMode) {};

	bool read(File& document);
};


PSAPI_NAMESPACE_END