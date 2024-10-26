#pragma once

#include "Macros.h"
#include "Core/Struct/File.h"

#include <string>
#include <array>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

PSAPI_NAMESPACE_BEGIN

// Represents a 4 byte wide signature that can be constructed from a uint32_t
struct Signature
{
	uint32_t m_Value{};
	std::array<char, 4> m_Representation{};

	Signature() : m_Value(0), m_Representation({' ', ' ', ' ', ' '}) {};
	Signature(const uint32_t val);
	Signature(const std::string val);

	bool operator==(const Signature& other);
	bool operator!=(const Signature& other);

	bool operator==(const std::string& other);
	bool operator!=(const std::string& other);

	bool operator==(const uint32_t other);
	bool operator!=(const uint32_t other);

	std::string string();

	// Read a 4-byte signature
	static Signature read(File& document);
	void write(File& document);
};


PSAPI_NAMESPACE_END