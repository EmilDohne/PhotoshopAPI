#pragma once

#include "FileHeader.h"
#include "Macros.h"
#include "Enum.h"
#include "Struct/File.h"
#include "Struct/Section.h"

#include <vector>

#include <cstdint>

PSAPI_NAMESPACE_BEGIN

struct ColorModeData : public FileSection
{
	std::vector<uint8_t> m_Data;

	ColorModeData() : m_Data({}) { m_Size = 4u; m_Offset = 26u; };
	// Note that we do not initialize any variables for FileSection here as that will be handled once we write the file
	ColorModeData(std::vector<uint8_t>& data) : m_Data(std::move(data)) {};

	uint64_t calculateSize(std::shared_ptr<FileHeader> header = nullptr) const override;

	void read(File& document);
	// Write the colorModeData section, note that the m_Data field does not contain the length marker and we parse it explicitly
	void write(File& document, FileHeader& header);
};

PSAPI_NAMESPACE_END