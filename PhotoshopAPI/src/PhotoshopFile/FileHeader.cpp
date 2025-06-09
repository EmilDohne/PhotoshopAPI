#include "FileHeader.h"

#include "Macros.h"
#include "Logger.h"
#include "Core/FileIO/Read.h"
#include "Core/FileIO/Write.h"
#include "Core/FileIO/BytesIO.h"

#include "Profiling/Perf/Instrumentor.h"

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

PSAPI_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void FileHeader::read(File& document)
{
	PSAPI_PROFILE_FUNCTION();

	FileSection::initialize(0, 26u);

	uint32_t signature = ReadBinaryData<uint32_t>(document);
	m_Signature = Signature(signature);
	if (signature != Signature("8BPS").m_Value)
	{
		PSAPI_LOG_ERROR("FileHeader", "Signature does not match 8BPS, got '%s' instead", m_Signature.m_Representation);
	}

	uint16_t version = ReadBinaryData<uint16_t>(document);
	try
	{
		m_Version = Enum::versionMap.at(version);
	}
	catch (const std::out_of_range& oor)
	{
		PSAPI_UNUSED(oor);
		PSAPI_LOG_ERROR("FileHeader", "Signature is not 1 or 2, got %" PRIu16 " instead", version);
	}

	// Skip reserved filler bytes
	document.skip(6u);

	m_NumChannels = ReadBinaryData<uint16_t>(document);
	if (m_NumChannels < 1u || m_NumChannels > 56u)
	{
		PSAPI_LOG_ERROR("FileHeader", "Number of channels is not between 1 and 56, got %" PRIu16 " instead", m_NumChannels);
	}


	m_Height = ReadBinaryData<uint32_t>(document);
	if (m_Version == Enum::Version::Psb)
	{
		if (m_Height < 1u || m_Height > 300000u)
		{
			PSAPI_LOG_ERROR("FileHeader", "Height is not between 1 and 300,000, got %" PRIu32 " instead", m_Height);
		}
	}
	else
	{
		if (m_Height < 1u || m_Height > 30000u)
		{
			PSAPI_LOG_ERROR("FileHeader", "Height is not between 1 and 30,000, got %" PRIu32 " instead", m_Height);
		}
	}

	m_Width = ReadBinaryData<uint32_t>(document);
	if (m_Version == Enum::Version::Psb)
	{
		if (m_Width < 1u || m_Width > 300000u)
		{
			PSAPI_LOG_ERROR("FileHeader", "Width is not between 1 and 300,000, got %" PRIu32 " instead", m_Width);
		}
	}
	else
	{
		if (m_Width < 1u || m_Width > 30000u)
		{
			PSAPI_LOG_ERROR("FileHeader", "Width is not between 1 and 30,000, got %" PRIu32 " instead", m_Width);
		}
	}


	uint16_t depth = ReadBinaryData<uint16_t>(document);
	try
	{
		m_Depth = Enum::bitDepthMap.at(depth);
	}
	catch (const std::out_of_range& oor)
	{
		PSAPI_UNUSED(oor);
		PSAPI_LOG_ERROR("FileHeader", "Depth is invalid, got %" PRIu16, depth);
	};

	uint16_t colorMode = ReadBinaryData<uint16_t>(document);
	try
	{
		m_ColorMode = Enum::colorModeMap.at(colorMode);
	}
	catch (const std::out_of_range& oor)
	{
		PSAPI_UNUSED(oor);
		PSAPI_LOG_ERROR("FileHeader", "ColorMode is invalid, got %" PRIu16, colorMode);
	};
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
FileHeader FileHeader::from_bytes(const std::vector<uint8_t>& file_data)
{
	PSAPI_PROFILE_FUNCTION();
	auto data_span = std::span<const uint8_t>(file_data.data(), file_data.size());
	size_t offset = 0;

	auto signature = Signature(bytes_io::read_as_and_swap<uint32_t>(data_span, offset));
	offset += 4u;
	if (signature != Signature("8BPS").m_Value)
	{
		PSAPI_LOG_ERROR("FileHeader", "Signature does not match 8BPS, got '%s' instead", signature.m_Representation);
	}

	uint16_t _version = bytes_io::read_as_and_swap<uint16_t>(data_span, offset);
	offset += 2u;
	Enum::Version version = Enum::Version::Psb;
	try
	{
		version = Enum::versionMap.at(_version);
	}
	catch ([[maybe_unused]] const std::out_of_range& oor)
	{
		PSAPI_LOG_ERROR("FileHeader", "Signature is not 1 or 2, got %" PRIu16 " instead", version);
	}

	// Skip reserved filler bytes
	offset += 6u;

	auto num_channels = bytes_io::read_as_and_swap<uint16_t>(data_span, offset);
	offset += 2u;
	if (num_channels < 1u || num_channels > 56u)
	{
		PSAPI_LOG_ERROR("FileHeader", "Number of channels is not between 1 and 56, got %" PRIu16 " instead", num_channels);
	}


	auto height  = bytes_io::read_as_and_swap<uint32_t>(data_span, offset);
	offset += 4u;
	if (version == Enum::Version::Psb)
	{
		if (height < 1u || height > 300000u)
		{
			PSAPI_LOG_ERROR("FileHeader", "Height is not between 1 and 300,000, got %" PRIu32 " instead", height);
		}
	}
	else
	{
		if (height < 1u || height > 30000u)
		{
			PSAPI_LOG_ERROR("FileHeader", "Height is not between 1 and 30,000, got %" PRIu32 " instead", height);
		}
	}

	auto width = bytes_io::read_as_and_swap<uint32_t>(data_span, offset);
	offset += 4u;
	if (version == Enum::Version::Psb)
	{
		if (width < 1u || width > 300000u)
		{
			PSAPI_LOG_ERROR("FileHeader", "Width is not between 1 and 300,000, got %" PRIu32 " instead", width);
		}
	}
	else
	{
		if (width < 1u || width > 30000u)
		{
			PSAPI_LOG_ERROR("FileHeader", "Width is not between 1 and 30,000, got %" PRIu32 " instead", width);
		}
	}


	uint16_t _depth = bytes_io::read_as_and_swap<uint16_t>(data_span, offset);
	offset += 2u;
	Enum::BitDepth depth = Enum::BitDepth::BD_8;
	try
	{
		depth = Enum::bitDepthMap.at(_depth);
	}
	catch ([[maybe_unused]] const std::out_of_range& oor)
	{
		PSAPI_LOG_ERROR("FileHeader", "Depth is invalid, got %" PRIu16, _depth);
	};

	uint16_t _color_mode = bytes_io::read_as_and_swap<uint16_t>(data_span, offset);
	offset += 2u;
	Enum::ColorMode color_mode = Enum::ColorMode::RGB;
	try
	{
		color_mode = Enum::colorModeMap.at(_color_mode);
	}
	catch ([[maybe_unused]] const std::out_of_range& oor)
	{
		PSAPI_LOG_ERROR("FileHeader", "ColorMode is invalid, got %" PRIu16, _color_mode);
	};

	return FileHeader(version, num_channels, width, height, depth, color_mode);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void FileHeader::write(File& document)
{
	PSAPI_PROFILE_FUNCTION();

	FileSection::initialize(0, 26u);

	// Write the signature, must be 8BPS
	WriteBinaryData<uint32_t>(document, Signature("8BPS").m_Value);

	// We automatically detect and write the version based on which extension our document has for simplicity's sake
	auto filePath = document.getPath();
	auto extension = filePath.extension();
	if (extension == ".psb")
	{
		m_Version = Enum::Version::Psb;
		std::optional<uint16_t> versionVal = findByValue(Enum::versionMap, Enum::Version::Psb);
		WriteBinaryData<uint16_t>(document, versionVal.value());
	}
	else if (extension == ".psd")
	{
		m_Version = Enum::Version::Psd;
		std::optional<uint16_t> versionVal = findByValue(Enum::versionMap, Enum::Version::Psd);
		WriteBinaryData<uint16_t>(document, versionVal.value());
	}
	else [[unlikely]]
	{
		PSAPI_LOG_ERROR("FileHeader", "Unable to deduce header version from extension, expected '.psb' or '.psd' but instead got %s", extension.string().c_str());
	}

	// Filler bytes, must be explicitly set them to 0
	WritePadddingBytes(document, 6u);

	WriteBinaryData<uint16_t>(document, m_NumChannels);

	WriteBinaryData<uint32_t>(document, m_Height);
	WriteBinaryData<uint32_t>(document, m_Width);

	std::optional<uint16_t> depthVal = findByValue(Enum::bitDepthMap, m_Depth);
	WriteBinaryData<uint16_t>(document, depthVal.value());

	std::optional<uint16_t> colorModeVal = findByValue(Enum::colorModeMap, m_ColorMode);
	WriteBinaryData<uint16_t>(document, colorModeVal.value());
}

PSAPI_NAMESPACE_END