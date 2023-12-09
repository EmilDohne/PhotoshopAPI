#pragma once

#include "Macros.h"

#include <string>
#include <sstream>
#include <vector>


PSAPI_NAMESPACE_BEGIN

// Convert a big endian uint32_t to a std::string. For example
inline std::string uint32ToString(const uint32_t value) {
    std::string charString;

    // Iterate through the bits, 8 bits at a time (each character)
    for (int i = 24; i >= 0; i -= 8) {
        char character = static_cast<char>((value >> i) & 0xFF);
        charString += character;
    }

    return charString;
}

// Split a string based on the given separator and return a vector of strings
inline std::vector<std::string> splitString(std::string toSplit, char separator)
{
    std::stringstream stream(toSplit);
    std::string segment;
    std::vector<std::string> segments;

    while (std::getline(stream, segment, separator))
    {
        segments.push_back(segment);
    }

    return segments;
}


PSAPI_NAMESPACE_END