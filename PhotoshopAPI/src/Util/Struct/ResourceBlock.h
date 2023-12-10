#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Struct/PascalString.h"
#include "Struct/File.h"
#include "Struct/Signature.h"

#include <vector>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>


PSAPI_NAMESPACE_BEGIN

struct ResourceBlock
{
	Enum::ImageResource m_UniqueId;
	PascalString m_Name;
	uint32_t m_Size = 0;				// Size of m_Data
	uint32_t m_BlockSize = 0;			// Size of the whole block

	std::vector<uint8_t> m_Data;

	ResourceBlock() : m_UniqueId(Enum::ImageResource::NotImplemented), m_Size(0), m_BlockSize(0) {};

	void read(File& document);
};

PSAPI_NAMESPACE_END