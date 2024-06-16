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
    // Covers all the conversions in the range of 128-255 as these are not mapped to UTF-8.
    // The rest of the range of 0-127 is same as ascii which is utf-8 compliant 
    inline static bidirectional_unordered_map<char, std::string> Windows1252_UTF8
    {
        {static_cast<char>(0x80), "\xe2\x82\xac"},
        {static_cast<char>(0x81), "\xef\xbf\xbd"},
        {static_cast<char>(0x82), "\xe2\x80\x9a"},
        {static_cast<char>(0x83), "\xc6\x92"},    
        {static_cast<char>(0x84), "\xe2\x80\x9e"},
        {static_cast<char>(0x85), "\xe2\x80\xa6"},
        {static_cast<char>(0x86), "\xe2\x80\xa0"},
        {static_cast<char>(0x87), "\xe2\x80\xa1"},
        {static_cast<char>(0x88), "\xcb\x86"},    
        {static_cast<char>(0x89), "\xe2\x80\xb0"},
        {static_cast<char>(0x8a), "\xc5\xa0"},    
        {static_cast<char>(0x8b), "\xe2\x80\xb9"},
        {static_cast<char>(0x8c), "\xc5\x92"},    
        {static_cast<char>(0x8d), "\xef\xbf\xbd"},
        {static_cast<char>(0x8e), "\xc5\xbd"},    
        {static_cast<char>(0x8f), "\xef\xbf\xbd"},
        {static_cast<char>(0x90), "\xef\xbf\xbd"},
        {static_cast<char>(0x91), "\xe2\x80\x98"},
        {static_cast<char>(0x92), "\xe2\x80\x99"},
        {static_cast<char>(0x93), "\xe2\x80\x9c"},
        {static_cast<char>(0x94), "\xe2\x80\x9d"},
        {static_cast<char>(0x95), "\xe2\x80\xa2"},  
        {static_cast<char>(0x96), "\xe2\x80\x93"},
        {static_cast<char>(0x97), "\xe2\x80\x94"},
        {static_cast<char>(0x98), "\xcb\x9c"},
        {static_cast<char>(0x99), "\xe2\x84\xa2"},
        {static_cast<char>(0x9a), "\xc5\xa1"},
        {static_cast<char>(0x9b), "\xe2\x80\xba"},
        {static_cast<char>(0x9c), "\xc5\x93"},
        {static_cast<char>(0x9d), "\xef\xbf\xbd"},
        {static_cast<char>(0x9e), "\xc5\xbe"},
        {static_cast<char>(0x9f), "\xc5\xb8"},
        {static_cast<char>(0xa0), "\xc2\xa0"},
        {static_cast<char>(0xa1), "\xc2\xa1"},
        {static_cast<char>(0xa2), "\xc2\xa2"},
        {static_cast<char>(0xa3), "\xc2\xa3"},
        {static_cast<char>(0xa4), "\xc2\xa4"},
        {static_cast<char>(0xa5), "\xc2\xa5"},
        {static_cast<char>(0xa6), "\xc2\xa6"},
        {static_cast<char>(0xa7), "\xc2\xa7"},
        {static_cast<char>(0xa8), "\xc2\xa8"},
        {static_cast<char>(0xa9), "\xc2\xa9"},
        {static_cast<char>(0xaa), "\xc2\xaa"},
        {static_cast<char>(0xab), "\xc2\xab"},
        {static_cast<char>(0xac), "\xc2\xac"},
        {static_cast<char>(0xad), "\xc2\xad"},
        {static_cast<char>(0xae), "\xc2\xae"},
        {static_cast<char>(0xaf), "\xc2\xaf"},
        {static_cast<char>(0xb0), "\xc2\xb0"},
        {static_cast<char>(0xb1), "\xc2\xb1"},
        {static_cast<char>(0xb2), "\xc2\xb2"},
        {static_cast<char>(0xb3), "\xc2\xb3"},
        {static_cast<char>(0xb4), "\xc2\xb4"},
        {static_cast<char>(0xb5), "\xc2\xb5"},
        {static_cast<char>(0xb6), "\xc2\xb6"},
        {static_cast<char>(0xb7), "\xc2\xb7"},
        {static_cast<char>(0xb8), "\xc2\xb8"},
        {static_cast<char>(0xb9), "\xc2\xb9"},
        {static_cast<char>(0xba), "\xc2\xba"},
        {static_cast<char>(0xbb), "\xc2\xbb"},
        {static_cast<char>(0xbc), "\xc2\xbc"},
        {static_cast<char>(0xbd), "\xc2\xbd"},
        {static_cast<char>(0xbe), "\xc2\xbe"},
        {static_cast<char>(0xbf), "\xc2\xbf"},
        {static_cast<char>(0xc0), "\xc3\x80"},
        {static_cast<char>(0xc1), "\xc3\x81"},
        {static_cast<char>(0xc2), "\xc3\x82"},
        {static_cast<char>(0xc3), "\xc3\x83"},
        {static_cast<char>(0xc4), "\xc3\x84"},
        {static_cast<char>(0xc5), "\xc3\x85"},
        {static_cast<char>(0xc6), "\xc3\x86"},
        {static_cast<char>(0xc7), "\xc3\x87"},
        {static_cast<char>(0xc8), "\xc3\x88"},
        {static_cast<char>(0xc9), "\xc3\x89"},
        {static_cast<char>(0xca), "\xc3\x8a"},
        {static_cast<char>(0xcb), "\xc3\x8b"},
        {static_cast<char>(0xcc), "\xc3\x8c"},
        {static_cast<char>(0xcd), "\xc3\x8d"},
        {static_cast<char>(0xce), "\xc3\x8e"},
        {static_cast<char>(0xcf), "\xc3\x8f"},
        {static_cast<char>(0xd0), "\xc3\x90"},
        {static_cast<char>(0xd1), "\xc3\x91"},
        {static_cast<char>(0xd2), "\xc3\x92"},
        {static_cast<char>(0xd3), "\xc3\x93"},
        {static_cast<char>(0xd4), "\xc3\x94"},
        {static_cast<char>(0xd5), "\xc3\x95"},
        {static_cast<char>(0xd6), "\xc3\x96"},
        {static_cast<char>(0xd7), "\xc3\x97"},
        {static_cast<char>(0xd8), "\xc3\x98"},
        {static_cast<char>(0xd9), "\xc3\x99"},
        {static_cast<char>(0xda), "\xc3\x9a"},
        {static_cast<char>(0xdb), "\xc3\x9b"},
        {static_cast<char>(0xdc), "\xc3\x9c"},
        {static_cast<char>(0xdd), "\xc3\x9d"},
        {static_cast<char>(0xde), "\xc3\x9e"},
        {static_cast<char>(0xdf), "\xc3\x9f"},
        {static_cast<char>(0xe0), "\xc3\xa0"},
        {static_cast<char>(0xe1), "\xc3\xa1"},
        {static_cast<char>(0xe2), "\xc3\xa2"},
        {static_cast<char>(0xe3), "\xc3\xa3"},
        {static_cast<char>(0xe4), "\xc3\xa4"},
        {static_cast<char>(0xe5), "\xc3\xa5"},
        {static_cast<char>(0xe6), "\xc3\xa6"},
        {static_cast<char>(0xe7), "\xc3\xa7"},
        {static_cast<char>(0xe8), "\xc3\xa8"},
        {static_cast<char>(0xe9), "\xc3\xa9"},
        {static_cast<char>(0xea), "\xc3\xaa"},
        {static_cast<char>(0xeb), "\xc3\xab"},
        {static_cast<char>(0xec), "\xc3\xac"},
        {static_cast<char>(0xed), "\xc3\xad"},
        {static_cast<char>(0xee), "\xc3\xae"},
        {static_cast<char>(0xef), "\xc3\xaf"},
        {static_cast<char>(0xf0), "\xc3\xb0"},
        {static_cast<char>(0xf1), "\xc3\xb1"},
        {static_cast<char>(0xf2), "\xc3\xb2"},
        {static_cast<char>(0xf3), "\xc3\xb3"},
        {static_cast<char>(0xf4), "\xc3\xb4"},
        {static_cast<char>(0xf5), "\xc3\xb5"},
        {static_cast<char>(0xf6), "\xc3\xb6"},
        {static_cast<char>(0xf7), "\xc3\xb7"},
        {static_cast<char>(0xf8), "\xc3\xb8"},
        {static_cast<char>(0xf9), "\xc3\xb9"},
        {static_cast<char>(0xfa), "\xc3\xba"},
        {static_cast<char>(0xfb), "\xc3\xbb"},
        {static_cast<char>(0xfc), "\xc3\xbc"},
        {static_cast<char>(0xfd), "\xc3\xbd"},
        {static_cast<char>(0xfe), "\xc3\xbe"},
        {static_cast<char>(0xff), "\xc3\xbf"}
    };


    // Covers all the conversions in the range of 128-255 as these are not mapped to UTF-8.
    // The rest of the range of 0-127 is same as ascii which is utf-8 compliant 
	inline static bidirectional_unordered_map<char, std::string> MacRoman_UTF8
	{
        {static_cast<char>(0x80), "\xC3\x84"},       
        {static_cast<char>(0x81), "\xC3\x85"},       
        {static_cast<char>(0x82), "\xC3\x87"},       
        {static_cast<char>(0x83), "\xC3\x89"},       
        {static_cast<char>(0x84), "\xC3\x91"},       
        {static_cast<char>(0x85), "\xC3\x96"},       
        {static_cast<char>(0x86), "\xC3\x9C"},       
        {static_cast<char>(0x87), "\xC3\xA1"},       
        {static_cast<char>(0x88), "\xC3\xA0"},       
        {static_cast<char>(0x89), "\xC3\xA2"},       
        {static_cast<char>(0x8A), "\xC3\xA4"},       
        {static_cast<char>(0x8B), "\xC3\xA3"},       
        {static_cast<char>(0x8C), "\xC3\xA5"},       
        {static_cast<char>(0x8D), "\xC3\xA7"},       
        {static_cast<char>(0x8E), "\xC3\xA9"},       
        {static_cast<char>(0x8F), "\xC3\xA8"},       
        {static_cast<char>(0x90), "\xC3\xAA"},       
        {static_cast<char>(0x91), "\xC3\xAB"},       
        {static_cast<char>(0x92), "\xC3\xAD"},       
        {static_cast<char>(0x93), "\xC3\xAC"},       
        {static_cast<char>(0x94), "\xC3\xAE"},       
        {static_cast<char>(0x95), "\xC3\xAF"},       
        {static_cast<char>(0x96), "\xC3\xB1"},       
        {static_cast<char>(0x97), "\xC3\xB3"},       
        {static_cast<char>(0x98), "\xC3\xB2"},       
        {static_cast<char>(0x99), "\xC3\xB4"},       
        {static_cast<char>(0x9A), "\xC3\xB6"},       
        {static_cast<char>(0x9B), "\xC3\xB5"},       
        {static_cast<char>(0x9C), "\xC3\xBA"},       
        {static_cast<char>(0x9D), "\xC3\xB9"},       
        {static_cast<char>(0x9E), "\xC3\xBB"},       
        {static_cast<char>(0x9F), "\xC3\xBC"},       
        {static_cast<char>(0xA0), "\xE2\x80\xA0"},   
        {static_cast<char>(0xA1), "\xC2\xB0"},       
        {static_cast<char>(0xA2), "\xC2\xA2"},       
        {static_cast<char>(0xA3), "\xC2\xA3"},       
        {static_cast<char>(0xA4), "\xC2\xA7"},       
        {static_cast<char>(0xA5), "\xE2\x80\xA2"},   
        {static_cast<char>(0xA6), "\xC2\xB6"},       
        {static_cast<char>(0xA7), "\xC3\x9F"},       
        {static_cast<char>(0xA8), "\xC2\xAE"},       
        {static_cast<char>(0xA9), "\xC2\xA9"},       
        {static_cast<char>(0xAA), "\xE2\x84\xA2"},   
        {static_cast<char>(0xAB), "\xC2\xB4"},       
        {static_cast<char>(0xAC), "\xC2\xA8"},       
        {static_cast<char>(0xAD), "\xE2\x89\xA0"},   
        {static_cast<char>(0xAE), "\xC3\x86"},       
        {static_cast<char>(0xAF), "\xC3\x98"},       
        {static_cast<char>(0xB0), "\xE2\x88\x9E"},   
        {static_cast<char>(0xB1), "\xC2\xB1"},       
        {static_cast<char>(0xB2), "\xE2\x89\xA4"},   
        {static_cast<char>(0xB3), "\xE2\x89\xA5"},   
        {static_cast<char>(0xB4), "\xC2\xA5"},       
        {static_cast<char>(0xB5), "\xC2\xB5"},       
        {static_cast<char>(0xB6), "\xE2\x88\x82"},   
        {static_cast<char>(0xB7), "\xE2\x88\x91"},   
        {static_cast<char>(0xB8), "\xE2\x88\x8F"},   
        {static_cast<char>(0xB9), "\xCF\x80"},       
        {static_cast<char>(0xBA), "\xE2\x88\xAB"},   
        {static_cast<char>(0xBB), "\xC2\xAA"},       
        {static_cast<char>(0xBC), "\xC2\xBA"},       
        {static_cast<char>(0xBD), "\xCE\xA9"},       
        {static_cast<char>(0xBE), "\xC3\xA6"},       
        {static_cast<char>(0xBF), "\xC3\xB8"},       
        {static_cast<char>(0xC0), "\xC2\xBF"},       
        {static_cast<char>(0xC1), "\xC2\xA1"},       
        {static_cast<char>(0xC2), "\xC2\xAC"},       
        {static_cast<char>(0xC3), "\xE2\x88\x9A"},   
        {static_cast<char>(0xC4), "\xC6\x92"},       
        {static_cast<char>(0xC5), "\xE2\x89\x88"},   
        {static_cast<char>(0xC6), "\xE2\x88\x86"},   
        {static_cast<char>(0xC7), "\xC2\xAB"},       
        {static_cast<char>(0xC8), "\xC2\xBB"},       
        {static_cast<char>(0xC9), "\xE2\x80\xA6"},   
        {static_cast<char>(0xCA), "\xC2\xA0"},       
        {static_cast<char>(0xCB), "\xC3\x80"},       
        {static_cast<char>(0xCC), "\xC3\x83"},       
        {static_cast<char>(0xCD), "\xC3\x95"},       
        {static_cast<char>(0xCE), "\xC5\x92"},       
        {static_cast<char>(0xCF), "\xC5\x93"},       
        {static_cast<char>(0xD0), "\xE2\x80\x93"},   
        {static_cast<char>(0xD1), "\xE2\x80\x94"},   
        {static_cast<char>(0xD2), "\xE2\x80\x9C"},   
        {static_cast<char>(0xD3), "\xE2\x80\x9D"},   
        {static_cast<char>(0xD4), "\xE2\x80\x98"},   
        {static_cast<char>(0xD5), "\xE2\x80\x99"},   
        {static_cast<char>(0xD6), "\xC3\xB7"},       
        {static_cast<char>(0xD7), "\xE2\x97\x8A"},   
        {static_cast<char>(0xD8), "\xC3\xBF"},       
        {static_cast<char>(0xD9), "\xC5\xB8"},       
        {static_cast<char>(0xDA), "\xE2\x81\x84"},   
        {static_cast<char>(0xDB), "\xE2\x82\xAC"},   
        {static_cast<char>(0xDC), "\xE2\x80\xB9"},   
        {static_cast<char>(0xDD), "\xE2\x80\xBA"},   
        {static_cast<char>(0xDE), "\xEF\xAC\x81"},   
        {static_cast<char>(0xDF), "\xEF\xAC\x82"},   
        {static_cast<char>(0xE0), "\xE2\x80\xA1"},   
        {static_cast<char>(0xE1), "\xC2\xB7"},       
        {static_cast<char>(0xE2), "\xE2\x80\x9A"},   
        {static_cast<char>(0xE3), "\xE2\x80\x9E"},   
        {static_cast<char>(0xE4), "\xE2\x80\xB0"},   
        {static_cast<char>(0xE5), "\xC3\x82"},       
        {static_cast<char>(0xE6), "\xC3\x8A"},       
        {static_cast<char>(0xE7), "\xC3\x81"},       
        {static_cast<char>(0xE8), "\xC3\x8B"},       
        {static_cast<char>(0xE9), "\xC3\x88"},       
        {static_cast<char>(0xEA), "\xC3\x8D"},       
        {static_cast<char>(0xEB), "\xC3\x8E"},       
        {static_cast<char>(0xEC), "\xC3\x8F"},       
        {static_cast<char>(0xED), "\xC3\x8C"},       
        {static_cast<char>(0xEE), "\xC3\x93"},       
        {static_cast<char>(0xEF), "\xC3\x94"},       
        {static_cast<char>(0xF0), "\xEF\xA3\xBF"},   
        {static_cast<char>(0xF1), "\xC3\x92"},       
        {static_cast<char>(0xF2), "\xC3\x9A"},       
        {static_cast<char>(0xF3), "\xC3\x9B"},       
        {static_cast<char>(0xF4), "\xC3\x99"},       
        {static_cast<char>(0xF5), "\xC4\xB1"},       
        {static_cast<char>(0xF6), "\xCB\x86"},       
        {static_cast<char>(0xF7), "\xCB\x9C"},       
        {static_cast<char>(0xF8), "\xC2\xAF"},       
        {static_cast<char>(0xF9), "\xCB\x98"},       
        {static_cast<char>(0xFA), "\xCB\x99"},       
        {static_cast<char>(0xFB), "\xCB\x9A"},       
        {static_cast<char>(0xFC), "\xC2\xB8"},       
        {static_cast<char>(0xFD), "\xCB\x9D"},       
        {static_cast<char>(0xFE), "\xCB\x9B"},       
        {static_cast<char>(0xFF), "\xCB\x87"},       
	};
}



/// Convert a single char from a given encoding to utf-8.
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
inline static std::string convertCharToUTF8(EncodingType encoding, const char character)
{
    switch (encoding)
    {
        case EncodingType::Windows_1252:
            if (static_cast<int>(character) >= 0x80)
                return Windows1252_UTF8[character];
            else
                return std::string(1, character);
        case EncodingType::Mac_Roman: 
            if (static_cast<int>(character) >= 0x80)
                return MacRoman_UTF8[character];
            else
                return std::string(1, character);
        default:
            PSAPI_LOG_WARNING("PascalString", "Unknown encoding encountered, skipping conversion");
            return std::string(1, character);
    }
}


/// Convert a string (e.g. a pascal string) from a given encoding into UTF-8. If the encoding
/// is unknown or not implemented we will raise a warning and return the input string
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
inline static std::string convertStrToUTF8(EncodingType encoding, const std::string str)
{
    PROFILE_FUNCTION();
    std::string res = "";
    for (const auto& character : str)
    {
        res += convertCharToUTF8(encoding, character);
    }
    return res;
}


/// Convert a utf-8 encoded string to another encoding. For the moment we simply map the ascii
/// characters and ignore any special character
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
inline static std::string ConvertUTF8ToStr(EncodingType encoding, const std::string str)
{
    PROFILE_FUNCTION();
    std::string res = "";
    for (const auto& character : str)
    {
        if (character < 0x80)
        {
            res += character;
        }
    }
    return res;
}



/// A pascal string in Photoshop terms refers to a char[] with a 1 byte preceding length marker
/// which includes the length marker itself. The length usually gets rounded up to a multiple of 2
/// or 4 bytes depending on which section its read from. The encoding matches that of the Pascal ShortString type which
/// can be e.g. MacOS Roman or Windows 1252. The decoding gets done at the boundary and the string is in UTF-8 format
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

	/// Return a view over the string held by this struct
	const std::string_view getStringView() const noexcept;

	void read(File& document, const uint8_t padding) noexcept;
	void write(File& document, const uint8_t padding) const;
private:
	std::string m_String;
};

PSAPI_NAMESPACE_END