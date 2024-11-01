#pragma once

#include "Macros.h"

#include <string>
#include <sstream>
#include <vector>
#include <random>

#include "stduuid/uuid.h"

PSAPI_NAMESPACE_BEGIN

// Convert a big endian uint32_t to a std::string
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


/// Generate a UUID using the stduuid library
inline std::string generateUUID()
{
    std::random_device rd;
    auto seed_data = std::array<int, std::mt19937::state_size> {};
    std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::mt19937 generator(seq);
    uuids::uuid_random_generator gen{ generator };

    return uuids::to_string(gen());
}


PSAPI_NAMESPACE_END