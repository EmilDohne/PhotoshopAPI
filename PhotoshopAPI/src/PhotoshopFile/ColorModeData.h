#pragma once

#include "../Macros.h"
#include "../Util/Enum.h"
#include "../Util/Struct/File.h"
#include "../Util/Struct/Section.h"
#include "FileHeader.h"

#include <vector>

#include <cstdint>

PSAPI_NAMESPACE_BEGIN

struct ColorModeData : public FileSection
{
	std::vector<char> m_Data;

	bool read(File& document, const FileHeader& header);
};

PSAPI_NAMESPACE_END