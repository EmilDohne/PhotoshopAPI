#pragma once

#include "Macros.h"
#include "File.h"

#include <optional>

#include <cstdint>

PSAPI_NAMESPACE_BEGIN

// Forward declare FileHeader to avoid any issues
struct FileHeader;

struct FileSection
{
	uint64_t m_Offset = 0u;
	uint64_t m_Size = 0u;	// Store the size of the whole section (including the length marker if applicable)

	// Each File Section must implement a way to calculate its own section size in bytes based on the 
	// data it holds. This is important for writing to disk as we sometimes hold a section size marker.
	// The size must include this marker (if applicable) and when writing the marker we subtract the size of it.
	// It is the responsibility of the constructor to call this function or any function that modifies the data,
	// but not the write functionality
	virtual uint64_t calculateSize(std::shared_ptr<FileHeader> header = nullptr) const  = 0;
};


PSAPI_NAMESPACE_END