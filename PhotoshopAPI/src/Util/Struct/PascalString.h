#pragma once

#include "Macros.h"
#include "Read.h"
#include "Struct/File.h"

#include <string>

PSAPI_NAMESPACE_BEGIN

// A pascal string in Photoshop terms refers to a char[] with a 1 byte preceding length marker
// which includes the length marker itself. The length usually gets rounded up to a multiple of 2
// or 4 bytes depending on which section its read from
struct PascalString
{
	uint8_t m_Size;	// Stores the padded length including the size marker
	std::string m_String;
	
	PascalString() : m_Size(0) {};
	// Initialize a padded PascalString based on its size
	PascalString(std::string name, const uint8_t padding);

	void read(File& document, const uint8_t padding);
};

PSAPI_NAMESPACE_END