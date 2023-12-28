#pragma once

#include "Macros.h"
#include "FileIO/Read.h"
#include "Struct/File.h"
#include "Struct/Section.h"

#include <string>

PSAPI_NAMESPACE_BEGIN

// A pascal string in Photoshop terms refers to a char[] with a 1 byte preceding length marker
// which includes the length marker itself. The length usually gets rounded up to a multiple of 2
// or 4 bytes depending on which section its read from
struct PascalString : public FileSection
{
	std::string m_String;
	
	PascalString() { FileSection::m_Size = 0u; };
	// Initialize a padded PascalString based on its size
	PascalString(std::string name, const uint8_t padding);

	// While we return a uint64_t here we actually make sure that the size does not exceed the size of uint8_t as that would be illegal
	uint64_t calculateSize(std::optional<FileHeader> header = std::nullopt) const override;

	void read(File& document, const uint8_t padding);
	void write(File& document, const uint8_t padding) const;
};

PSAPI_NAMESPACE_END