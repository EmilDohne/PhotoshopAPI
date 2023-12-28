#pragma once

#include "Macros.h"
#include "File.h"
#include "PhotoshopFile/FileHeader.h"

#include <optional>

#include <cstdint>

PSAPI_NAMESPACE_BEGIN

struct FileSection
{
	uint64_t m_Offset;
	uint64_t m_Size;	// Store the size of the whole section (including the length marker if applicable)

	// Each File Section must implement a way to calculate its own section size in bytes based on the 
	// data it holds. This is important for writing to disk as we sometimes hold a section size marker.
	// The size must include this marker (if applicable) and when writing the marker we subtract the size of it.
	// It is the responsibility of the constructor to call this function or any function that modifies the data,
	// but not the write functionality
	virtual uint64_t calculateSize(std::optional<FileHeader> header = std::nullopt) const = 0;
};


PSAPI_NAMESPACE_END