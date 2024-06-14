#pragma once

#include "Macros.h"
#include "Core/FileIO/Read.h"
#include "Core/Struct/File.h"
#include "Core/Struct/Section.h"

#include <string>

PSAPI_NAMESPACE_BEGIN

/// A pascal string in Photoshop terms refers to a char[] with a 1 byte preceding length marker
/// which includes the length marker itself. The length usually gets rounded up to a multiple of 2
/// or 4 bytes depending on which section its read from. The encoding appears to be UTF-8
struct PascalString : public FileSection
{
	PascalString() { FileSection::m_Size = 2u; m_String = ""; };
	// Initialize a padded PascalString based on its size
	PascalString(std::string name, const uint8_t padding);

	// While we return a uint64_t here we actually make sure that the size does not exceed the size of uint8_t as that would be illegal
	uint64_t calculateSize(std::shared_ptr<FileHeader> header = nullptr) const override;

	/// Return the string held by this struct, the string is ASCII and therefore 
	/// UTF8 compliant
	std::string getString() const noexcept;

	/// Return a view over the string held by this struct, the string is ASCII and
	/// therefore UTF8 compliant
	const std::string_view getStringView() const noexcept;

	void read(File& document, const uint8_t padding) noexcept;
	void write(File& document, const uint8_t padding) const;
private:
	std::string m_String;
};

PSAPI_NAMESPACE_END