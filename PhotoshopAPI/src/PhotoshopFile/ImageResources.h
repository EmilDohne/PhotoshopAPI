#pragma once

#include "../Macros.h"
#include "../Util/Enum.h"
#include "../Util/Struct/File.h"
#include "../Util/Struct/Section.h"
#include "../Util/Struct/ResourceBlock.h"

#include <vector>

#include <cstdint>

PSAPI_NAMESPACE_BEGIN

struct ImageResources : public FileSection
{
	std::vector<ResourceBlock> m_ResourceBlocks;

	bool read(File& document, const uint64_t offset);
};


PSAPI_NAMESPACE_END