#pragma once

#include "Macros.h"
#include "Core/FileIO/Read.h"
#include "Core/Struct/Section.h"
#include "Core/Struct/File.h"

#include "simdutf.h"

#include <string>

PSAPI_NAMESPACE_BEGIN

/// A Unicode string as defined in the Photoshop File format is a UTF-16 Big Endian 
/// byte buffer preceded by a 4-byte count which refers to the number of code points (so to get the number of
/// bytes we must multiply by 2).
/// 
/// While the documentation mentions that the UnicodeString must end in a two byte null in reality this is often
/// untrue and the section is instead simpy padded. For example the 'luni' Tagged block is aligned to 4 bytes
/// 
/// We parse this internally into a UTF-8 encoded std::string as well as storing the original UTF-16BE string
struct UnicodeString : public FileSection
{
	/// Initialize an empty section.
	UnicodeString() { FileSection::size(4u); m_String = ""; m_UTF16String = {}; };

	/// Construct a UnicodeString from the given UTF8 encoded string aligning 
	/// the section to the given padding.
	UnicodeString(std::string str, const uint8_t padding);

	/// This method returns the absolute size of the UnicodeString in bytes including the 4-byte size
	uint64_t calculateSize(std::shared_ptr<FileHeader> header = nullptr) const override;

	/// Retrieve the UTF8 representation of the struct
	const std::string getString() const noexcept;
	const std::string_view getStringView() const noexcept;
	/// Retrieve the UTF16LE representation of the struct
	std::u16string getUTF16String() const noexcept;
	
	/// Static conversion functions between UTF16 <-> UTF8 for use outside
	/// of the PhotoshopAPI. This function checks the incoming string 
	/// for correctness and validity
	/// \param str the string to convert, 
	static std::u16string convertUTF8ToUTF16LE(const std::string& str);
	static std::u16string convertUTF8ToUTF16BE(const std::string& str);
	static std::string convertUTF16LEtoUTF8(const std::u16string& str);
	static std::string convertUTF16BEtoUTF8(const std::u16string& str);

	/// Read a Photoshop UnicodeString struct storing both the UTF8 and UTF16LE
	/// representation
	void read(File& document, const uint8_t padding);

	/// Write the stored UTF16 string to disk with a 4-byte length with the padding
	/// defined in the constructor
	void write(File& document) const;
private:
	// UTF-8 representation of the string
	std::string m_String;
	// UTF-16 LE representation of the string
	std::u16string m_UTF16String;
};


PSAPI_NAMESPACE_END