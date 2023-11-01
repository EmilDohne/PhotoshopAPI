#pragma once

#include "../Macros.h"

#include <string>


PSAPI_NAMESPACE_BEGIN

// Convert a big endian uint32_t to a std::string. For example
std::string uint32ToString(const uint32_t value) {
    std::string charString;

    // Iterate through the bits, 8 bits at a time (each character)
    for (int i = 24; i >= 0; i -= 8) {
        char character = static_cast<char>((value >> i) & 0xFF);
        charString += character;
    }

    return charString;
}


PSAPI_NAMESPACE_END