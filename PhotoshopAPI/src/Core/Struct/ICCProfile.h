#pragma once

#include "Macros.h"

#include <vector>
#include <filesystem>

PSAPI_NAMESPACE_BEGIN

/// Helper Structure for loading an ICC profile from memory of disk. Photoshop will then store
/// the raw bytes of the ICC profile in their ICCProfile ResourceBlock (ID 1039)
struct ICCProfile
{
	/// Initialize an empty ICCProfile
	ICCProfile() : m_Data({}) {};
	/// Initialize the ICCProfile by passing in a raw byte array of an ICC profile
	ICCProfile(std::vector<uint8_t> data) : m_Data(std::move(data)) {};
	/// Initialize the ICCProfile by loading the path contents from disk
	ICCProfile(const std::filesystem::path& pathToICCFile);

	/// Return a copy of the ICC profile data
	std::vector<uint8_t> data() const noexcept { return m_Data; };

	/// Return the absolute size of the data
	uint32_t data_size() const noexcept { return static_cast<uint32_t>(m_Data.size()); };

private:
	std::vector<uint8_t> m_Data;
};

PSAPI_NAMESPACE_END