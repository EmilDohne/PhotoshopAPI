#pragma once

#include "doctest.h"

#include <filesystem>

// function to read the files found in the compression folder with which we can continuously retest the same layer structure as well as easily expand
// on further test case
template <typename T>
inline void checkCompressionFile(std::filesystem::path& inputPath, const double zero_val, const double val_128, const double one_val);
