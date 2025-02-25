#pragma once

#include "Macros.h"
#include "Util/Enum.h"
#include "Core/Struct/File.h"
#include "Core/Struct/Signature.h"
#include "Core/Struct/Section.h"

#include <cstdint>


PSAPI_NAMESPACE_BEGIN


struct FileHeader : public FileSection
{
	/// Has to be '8BPS'
	Signature m_Signature{};
	/// The type of file we are dealing with
	Enum::Version m_Version{};
	/// Supported range by Photoshop is 1-56. This does not account for mask channels
	uint16_t m_NumChannels{};
	/// 1 - 30,000 for PSD and 1 - 300,000 for PSB
	uint32_t m_Height{};
	/// 1 - 30,000 for PSD and 1 - 300,000 for PSB
	uint32_t m_Width{};
	/// Depth of the Document
	Enum::BitDepth m_Depth{};
	/// Color Mode of the file
	Enum::ColorMode m_ColorMode{};

	FileHeader() = default;
	// Note that we do not initialize any variables for FileSection here as that will be handled once we write the file
	FileHeader(Enum::Version version, uint16_t numChannels, uint32_t width, uint32_t height, Enum::BitDepth depth, Enum::ColorMode colorMode) :
		m_Signature(Signature("8BPS")),
		m_Version(version),
		m_NumChannels(numChannels),
		m_Height(height),
		m_Width(width),
		m_Depth(depth),
		m_ColorMode(colorMode) {};

	/// Read and Initialize the FileHeader from disk
	void read(File& document);

	/// Write out the data held by the struct in a Photoshop compliant way
	void write(File& document);
};


PSAPI_NAMESPACE_END