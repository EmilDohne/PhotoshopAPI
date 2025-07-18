#pragma once

#include "FileHeader.h"
#include "Macros.h"
#include "Util/Enum.h"
#include "Core/Struct/File.h"
#include "Core/Struct/Section.h"

#include <vector>
#include <span>

#include <cstdint>

PSAPI_NAMESPACE_BEGIN

/// The ColorModeData section holds information for e.g. how the image is tonemapped in 32-bit mode as well as mapping of indexed and duotone
/// colours.
struct ColorModeData : public FileSection
{
	/// The raw bytes held by this section excluding the section marker. This should only hold data for 32-bit files and Duotone/Indexed
	/// color mode. For the time being we do not interpret this data in any way and defaults are written automatically
	std::vector<uint8_t> m_Data;

	ColorModeData() : m_Data({}) { FileSection::initialize(26u, 4u); };
	ColorModeData(std::vector<uint8_t>& data) : m_Data(std::move(data)) {};

	/// Read the ColorModeData section as is without interpreting anything
	void read(File& document);

	/// \brief Return the section size without parsing the whole struct.
	/// \param data_span The span starting from this section
	static size_t get_size(const std::span<const uint8_t> data_span);

	/// Write the ColorModeData section, note that the m_Data field does not contain the length marker and we write it explicitly
	void write(File& document, FileHeader& header);
};

PSAPI_NAMESPACE_END