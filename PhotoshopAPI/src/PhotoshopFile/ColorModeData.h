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

	bool read(File& document);
};

PSAPI_NAMESPACE_END