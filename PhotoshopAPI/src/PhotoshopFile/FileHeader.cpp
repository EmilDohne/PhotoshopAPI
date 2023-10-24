
#include "FileHeader.h"

#include "../Macros.h"
#include "../Util/Read.h"

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

PSAPI_NAMESPACE_BEGIN


bool FileHeader::read(File& document)
{
	m_Offset = 0;
	m_Size = 26;

	uint32_t signature = ReadBinaryData<uint32_t>(document);
	m_Signature = Signature(signature);
	if (m_Signature != Signature("8BPS"))
	{
		PSAPI_LOG_ERROR("FileHeader", "Signature does not match 8BPS, got '%s' instead", m_Signature);
		return false;
	}

	uint16_t version = ReadBinaryData<uint16_t>(document);
	try
	{
		m_Version = Enum::versionMap.at(version);
	}
	catch (const std::out_of_range& oor)
	{
		(void)oor;
		PSAPI_LOG_ERROR("FileHeader", "Signature is not 1 or 2, got %" PRIu16 " instead", version);
		return false;
	}

	// Skip reserved filler bytes
	document.skip(6u);

	m_NumChannels = ReadBinaryData<uint16_t>(document);
	if (m_NumChannels < 1u || m_NumChannels > 56u)
	{
		PSAPI_LOG_ERROR("FileHeader", "Number of channels is not between 1 and 56, got %" PRIu16 " instead", m_NumChannels);
		return false;
	}


	m_Height = ReadBinaryData<uint32_t>(document);
	if (m_Version == Enum::Version::Psb)
	{
		if (m_Height < 1u || m_Height > 300000u)
		{
			PSAPI_LOG_ERROR("FileHeader", "Height is not between 1 and 300,000, got %" PRIu32 " instead", m_Height);
			return false;
		}
	}
	else
	{
		if (m_Height < 1u || m_Height > 30000u)
		{
			PSAPI_LOG_ERROR("FileHeader", "Height is not between 1 and 30,000, got %" PRIu32 " instead", m_Height);
			return false;
		}
	}

	m_Width = ReadBinaryData<uint32_t>(document);
	if (m_Version == Enum::Version::Psb)
	{
		if (m_Width < 1u || m_Width > 300000u)
		{
			PSAPI_LOG_ERROR("FileHeader", "Width is not between 1 and 300,000, got %" PRIu32 " instead", m_Width);
			return false;
		}
	}
	else
	{
		if (m_Width < 1u || m_Width > 30000u)
		{
			PSAPI_LOG_ERROR("FileHeader", "Width is not between 1 and 30,000, got %" PRIu32 " instead", m_Width);
			return false;
		}
	}


	uint16_t depth = ReadBinaryData<uint16_t>(document);
	try
	{
		m_Depth = Enum::bitDepthMap.at(depth);
	}
	catch (const std::out_of_range& oor)
	{
		(void)oor;
		PSAPI_LOG_ERROR("FileHeader", "Depth is invalid, got %" PRIu16, depth);
		return false;
	};

	uint16_t colorMode = ReadBinaryData<uint16_t>(document);
	try
	{
		m_ColorMode = Enum::colorModeMap.at(colorMode);
	}
	catch (const std::out_of_range& oor)
	{
		(void)oor;
		PSAPI_LOG_ERROR("FileHeader", "ColorMode is invalid, got %" PRIu16, colorMode);
		return false;
	};

	return true;
}

PSAPI_NAMESPACE_END