#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Struct/PascalString.h"
#include "Struct/Section.h"
#include "Struct/File.h"
#include "Struct/Signature.h"

#include <vector>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>


PSAPI_NAMESPACE_BEGIN

struct ResourceBlock : public FileSection
{
	Enum::ImageResource m_UniqueId;
	PascalString m_Name;
	uint32_t m_DataSize = 0;				// Size of m_Data, padded to 2 bytes

	std::vector<uint8_t> m_Data;

	ResourceBlock() : m_UniqueId(Enum::ImageResource::NotImplemented), m_Name("", 2u), m_DataSize(0) { m_Data = {}; m_Size = this->calculateSize(); };
	
	// This calculates the size of m_Size only, not m_DataSize!
	uint64_t calculateSize(std::optional<FileHeader> header = std::nullopt) const override;

	void read(File& document);
	void write(File& document);
};

PSAPI_NAMESPACE_END