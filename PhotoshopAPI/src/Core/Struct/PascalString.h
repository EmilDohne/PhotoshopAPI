#pragma once

#include "Macros.h"
#include "Core/FileIO/Read.h"
#include "Core/Struct/File.h"
#include "Core/Struct/Section.h"
#include "Core/Struct/BidirectionalMap.h"

#include <string>
#include <unordered_map>

PSAPI_NAMESPACE_BEGIN


/// Encoding types used by PascalStrings to represent strings as a sequence of bytes.
/// On windows this appears to always be Windows_1252. These reprenset the CP_ACP 
/// type of PascalStrings but the exact encoding used on each system is unsure. It is furthermore
/// possible that different applications saving PSD/PSB might use a different encoding system
enum class EncodingType
{
	Windows_1252,
	Mac_Roman
};



namespace
{
	inline static bidirectional_unordered_map<char, std::string> Windows1252_UTF8
	{
        {}
        {static_cast<char>(0x80), "\xC3\x84"},       // A umlaut
        {static_cast<char>(0x81), "\xC3\x85"},       // A circle
        {static_cast<char>(0x82), "\xC3\x87"},       // C cedilla
        {static_cast<char>(0x83), "\xC3\x89"},       // E accent
        {static_cast<char>(0x84), "\xC3\x91"},       // N tilde
        {static_cast<char>(0x85), "\xC3\x96"},       // O umlaut
        {static_cast<char>(0x86), "\xC3\x9C"},       // U umlaut
        {static_cast<char>(0x87), "\xC3\xA1"},       // a accent
        {static_cast<char>(0x88), "\xC3\xA0"},       // a grave
        {static_cast<char>(0x89), "\xC3\xA2"},       // a circumflex
        {static_cast<char>(0x8A), "\xC3\xA4"},       // a umlaut
        {static_cast<char>(0x8B), "\xC3\xA3"},       // a tilde
        {static_cast<char>(0x8C), "\xC3\xA5"},       // a circle
        {static_cast<char>(0x8D), "\xC3\xA7"},       // c cedilla
        {static_cast<char>(0x8E), "\xC3\xA9"},       // e accent
        {static_cast<char>(0x8F), "\xC3\xA8"},       // e grave
        {static_cast<char>(0x90), "\xC3\xAA"},       // e circumflex
        {static_cast<char>(0x91), "\xC3\xAB"},       // e umlaut
        {static_cast<char>(0x92), "\xC3\xAD"},       // i accent
        {static_cast<char>(0x93), "\xC3\xAC"},       // i grave
        {static_cast<char>(0x94), "\xC3\xAE"},       // i circumflex
        {static_cast<char>(0x95), "\xC3\xAF"},       // i umlaut
        {static_cast<char>(0x96), "\xC3\xB1"},       // n tilde
        {static_cast<char>(0x97), "\xC3\xB3"},       // o accent
        {static_cast<char>(0x98), "\xC3\xB2"},       // o grave
        {static_cast<char>(0x99), "\xC3\xB4"},       // o circumflex
        {static_cast<char>(0x9A), "\xC3\xB6"},       // o umlaut
        {static_cast<char>(0x9B), "\xC3\xB5"},       // o tilde
        {static_cast<char>(0x9C), "\xC3\xBA"},       // u accent
        {static_cast<char>(0x9D), "\xC3\xB9"},       // u grave
        {static_cast<char>(0x9E), "\xC3\xBB"},       // u circumflex
        {static_cast<char>(0x9F), "\xC3\xBC"},       // u tilde
        {static_cast<char>(0xA0), "\xE2\x80\xA0"},   // cross
        {static_cast<char>(0xA1), "\xC2\xB0"},       // degree
        {static_cast<char>(0xA2), "\xC2\xA2"},       // cents
        {static_cast<char>(0xA3), "\xC2\xA3"},       // pounds
        {static_cast<char>(0xA4), "\xC2\xA7"},       // section
        {static_cast<char>(0xA5), "\xE2\x80\xA2"},   // bullet
        {static_cast<char>(0xA6), "\xC2\xB6"},       // pilcrow
        {static_cast<char>(0xA7), "\xC3\x9F"},       // german sharp S
        {static_cast<char>(0xA8), "\xC2\xAE"},       // registered
        {static_cast<char>(0xA9), "\xC2\xA9"},       // copyright
        {static_cast<char>(0xAA), "\xE2\x84\xA2"},   // TM
        {static_cast<char>(0xAB), "\xC2\xB4"},       // back tick
        {static_cast<char>(0xAC), "\xC2\xA8"},       // umlaut
        {static_cast<char>(0xAD), "\xE2\x89\xA0"},   // not equal (not in Windows 1252)
        {static_cast<char>(0xAE), "\xC3\x86"},       // AE
        {static_cast<char>(0xAF), "\xC3\x98"},       // O slash
        {static_cast<char>(0xB0), "\xE2\x88\x9E"},   // infinity (not in Windows 1252)
        {static_cast<char>(0xB1), "\xC2\xB1"},       // plus or minus
        {static_cast<char>(0xB2), "\xE2\x89\xA4"},   // less than or equal (not in Windows 1252)
        {static_cast<char>(0xB3), "\xE2\x89\xA5"},   // greater than or equal (not in Windows 1252)
        {static_cast<char>(0xB4), "\xC2\xA5"},       // yen
        {static_cast<char>(0xB5), "\xC2\xB5"},       // mu
        {static_cast<char>(0xB6), "\xE2\x88\x82"},   // derivative (not in Windows 1252)
        {static_cast<char>(0xB7), "\xE2\x88\x91"},   // large sigma (not in Windows 1252)
        {static_cast<char>(0xB8), "\xE2\x88\x8F"},   // large pi (not in Windows 1252)
        {static_cast<char>(0xB9), "\xCF\x80"},       // small pi (not in Windows 1252)
        {static_cast<char>(0xBA), "\xE2\x88\xAB"},   // integral (not in Windows 1252)
        {static_cast<char>(0xBB), "\xC2\xAA"},       // feminine ordinal
        {static_cast<char>(0xBC), "\xC2\xBA"},       // masculine ordinal
        {static_cast<char>(0xBD), "\xCE\xA9"},       // large ohm (not in Windows 1252)
        {static_cast<char>(0xBE), "\xC3\xA6"},       // ae
        {static_cast<char>(0xBF), "\xC3\xB8"},       // o slash
        {static_cast<char>(0xC0), "\xC2\xBF"},       // inverted question mark
        {static_cast<char>(0xC1), "\xC2\xA1"},       // inverted exclamation mark
        {static_cast<char>(0xC2), "\xC2\xAC"},       // not
        {static_cast<char>(0xC3), "\xE2\x88\x9A"},   // root (not in Windows 1252)
        {static_cast<char>(0xC4), "\xC6\x92"},       // function
        {static_cast<char>(0xC5), "\xE2\x89\x88"},   // approximately equal (not in Windows 1252)
        {static_cast<char>(0xC6), "\xE2\x88\x86"},   // large delta (not in Windows 1252)
        {static_cast<char>(0xC7), "\xC2\xAB"},       // open angle quotation mark
        {static_cast<char>(0xC8), "\xC2\xBB"},       // close angle quotation mark
        {static_cast<char>(0xC9), "\xE2\x80\xA6"},   // ellipsis
        {static_cast<char>(0xCA), "\xC2\xA0"},       // NBSP
        {static_cast<char>(0xCB), "\xC3\x80"},       // A grave
        {static_cast<char>(0xCC), "\xC3\x83"},       // A tilde
        {static_cast<char>(0xCD), "\xC3\x95"},       // O tilde
        {static_cast<char>(0xCE), "\xC5\x92"},       // OE
        {static_cast<char>(0xCF), "\xC5\x93"},       // oe
        {static_cast<char>(0xD0), "\xE2\x80\x93"},   // en dash
        {static_cast<char>(0xD1), "\xE2\x80\x94"},   // em dash
        {static_cast<char>(0xD2), "\xE2\x80\x9C"},   // open smart double quote
        {static_cast<char>(0xD3), "\xE2\x80\x9D"},   // close smart double quote
        {static_cast<char>(0xD4), "\xE2\x80\x98"},   // open smart single quote
        {static_cast<char>(0xD5), "\xE2\x80\x99"},   // close smart single quote
        {static_cast<char>(0xD6), "\xC3\xB7"},       // divided
        {static_cast<char>(0xD7), "\xE2\x97\x8A"},   // diamond (not in Windows 1252)
        {static_cast<char>(0xD8), "\xC3\xBF"},       // y umlaut
        {static_cast<char>(0xD9), "\xC5\xB8"},       // Y umlaut
        {static_cast<char>(0xDA), "\xE2\x81\x84"},   // big slash (not in Windows 1252)
        {static_cast<char>(0xDB), "\xE2\x82\xAC"},   // euro (not in Windows 1252)
        {static_cast<char>(0xDC), "\xE2\x80\xB9"},   // open angle single quote
        {static_cast<char>(0xDD), "\xE2\x80\xBA"},   // close angle single quote
        {static_cast<char>(0xDE), "\xEF\xAC\x81"},   // fi ligature (not in Windows 1252)
        {static_cast<char>(0xDF), "\xEF\xAC\x82"},   // fl ligature (not in Windows 1252)
        {static_cast<char>(0xE0), "\xE2\x80\xA1"},   // double dagger
        {static_cast<char>(0xE1), "\xC2\xB7"},       // interpunct
        {static_cast<char>(0xE2), "\xE2\x80\x9A"},   // inverted smart single quote
        {static_cast<char>(0xE3), "\xE2\x80\x9E"},   // inverted smart double quote
        {static_cast<char>(0xE4), "\xE2\x80\xB0"},   // per mille
        {static_cast<char>(0xE5), "\xC3\x82"},       // A circumflex
        {static_cast<char>(0xE6), "\xC3\x8A"},       // E circumflex
        {static_cast<char>(0xE7), "\xC3\x81"},       // A accent
        {static_cast<char>(0xE8), "\xC3\x8B"},       // E umlaut
        {static_cast<char>(0xE9), "\xC3\x88"},       // E grave
        {static_cast<char>(0xEA), "\xC3\x8D"},       // I accent
        {static_cast<char>(0xEB), "\xC3\x8E"},       // I circumflex
        {static_cast<char>(0xEC), "\xC3\x8F"},       // I umlaut
        {static_cast<char>(0xED), "\xC3\x8C"},       // I grave
        {static_cast<char>(0xEE), "\xC3\x93"},       // O accent
        {static_cast<char>(0xEF), "\xC3\x94"},       // O circumflex
        {static_cast<char>(0xF0), "\xEF\xA3\xBF"},   // box (not in Windows 1252)
        {static_cast<char>(0xF1), "\xC3\x92"},       // O grave
        {static_cast<char>(0xF2), "\xC3\x9A"},       // U accent
        {static_cast<char>(0xF3), "\xC3\x9B"},       // U circumflex
        {static_cast<char>(0xF4), "\xC3\x99"},       // U grave
        {static_cast<char>(0xF5), "\xC4\xB1"},       // dotless i ligature (not in Windows 1252)
        {static_cast<char>(0xF6), "\xCB\x86"},       // circumflex
        {static_cast<char>(0xF7), "\xCB\x9C"},       // tilde
        {static_cast<char>(0xF8), "\xC2\xAF"},       // macron
        {static_cast<char>(0xF9), "\xCB\x98"},       // breve (not in Windows 1252)
        {static_cast<char>(0xFA), "\xCB\x99"},       // raised dot (not in Windows 1252)
        {static_cast<char>(0xFB), "\xCB\x9A"},       // ring
        {static_cast<char>(0xFC), "\xC2\xB8"},       // cedilla
        {static_cast<char>(0xFD), "\xCB\x9D"},       // double acute accent (not in Windows 1252)
        {static_cast<char>(0xFE), "\xCB\x9B"},       // ogonek (not in Windows 1252)
        {static_cast<char>(0xFF), "\xCB\x87"},       // caron (not in Windows 1252)
	};


	inline static bidirectional_unordered_map<char, std::string> MacRoman_UTF8
	{

	};

	inline static std::string convertCharToUTF8(EncodingType encoding, const char character)
	{
		switch (encoding)
		{
		case EncodingType::Windows_1252: return Windows1252_UTF8[character];
		case EncodingType::Mac_Roman: return MacRoman_UTF8[character];
		default: 
			PSAPI_LOG_WARNING("PascalString", "Unknown encoding encountered, skipping conversion");
			return std::to_string(character);
		}
	}


	// Convert a string (e.g. a pascal string) from a given encoding into UTF-8. If the encoding
	// is unknown or not implemented we will raise a warning and return the input string
	inline static std::string convertStrToUTF8(EncodingType encoding, const std::string str)
	{
		PROFILE_FUNCTION();
		std::string res = "";
		for (const auto& character : str)
		{
			res += convertCharToUTF8(encoding, character);
		}
	}
}





/// A pascal string in Photoshop terms refers to a char[] with a 1 byte preceding length marker
/// which includes the length marker itself. The length usually gets rounded up to a multiple of 2
/// or 4 bytes depending on which section its read from. The encoding matches that of the Pascal ShortString type which
/// is supposedly 
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