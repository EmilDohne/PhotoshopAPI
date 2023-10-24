#pragma once

#include "../../Macros.h"

#include <cstdint>

PSAPI_NAMESPACE_BEGIN

struct FileSection
{
	uint64_t m_Offset;
	uint64_t m_Size;	// Store the size of the whole section (including the length marker if applicable)
};


PSAPI_NAMESPACE_END