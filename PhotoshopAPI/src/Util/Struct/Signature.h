#pragma once

#include "../../Macros.h"

#include <string>

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

PSAPI_NAMESPACE_BEGIN

// Represents a 4 byte wide signature that can be constructed from a uint32_t
struct Signature
{
	uint32_t m_Value;
	unsigned char m_Representation[4];

	Signature() : m_Value(0), m_Representation("") {};
	Signature(const uint32_t val);
	Signature(const std::string val);

	bool operator==(const Signature& other);
	bool operator!=(const Signature& other);
};


PSAPI_NAMESPACE_END