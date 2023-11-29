#pragma once

#include "doctest.h"

#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "Macros.h"

#include <filesystem>


// Implementation of the above function. This specialization is necessary due to the way in which we access the layerinfo for 8, 16 and 32 bit files
template <typename T>
void checkCompressionFileImpl(NAMESPACE_PSAPI::LayerInfo& layerInformation, const double zero_val, const double val_128, const double one_val, const double red_zero_val);


// function to read the files found in the compression folder with which we can continuously retest the same layer structure as well as easily expand
// on further test case.
// Note the red_zero_val parameter here. This is not a mistake. When saving out a completely red channel in 16-bit mode the other channels will actually be 
// of a value of 2. This is only for completely red pixels and I am unsure why this is the case but we do need the specialization
template <typename T>
void checkCompressionFile(std::filesystem::path& inputPath, const double zero_val, const double val_128, const double one_val, const double red_zero_val);
