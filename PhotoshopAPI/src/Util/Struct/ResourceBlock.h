#pragma once

#include "../../Macros.h"
#include "../Enum.h"
#include "PascalString.h"
#include "File.h"
#include "Signature.h"

#include <vector>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>


PSAPI_NAMESPACE_BEGIN

struct ResourceBlock
{
	Signature m_Signature;				// Must be 8BIM
	Enum::ImageResource m_UniqueId;
	PascalString m_Name;
	uint32_t m_Size = 0;				// Size of m_Data
	uint32_t m_BlockSize = 0;			// Size of the whole block

	std::vector<char> m_Data;

	ResourceBlock() : m_Size(0), m_BlockSize(0) {};
	ResourceBlock(File& document);
};

PSAPI_NAMESPACE_END