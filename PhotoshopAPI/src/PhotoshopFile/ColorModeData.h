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

	ColorModeData(): m_Data({}) {};
	// Note that we do not initialize any variables for FileSection here as that will be handled once we write the file
	ColorModeData(std::vector<uint8_t>& data) : m_Data(std::move(data)) {};

	bool read(File& document);
};

PSAPI_NAMESPACE_END