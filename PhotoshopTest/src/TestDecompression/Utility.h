/*
These functions test a specific type of file which are currently located under PhotoshopTest/documents/Compression/
If these were to change in the future to e.g. expand on the tests or improve upon them the checkDecompressionFileImpl() function would need
to be updated to implement these changes

This document is 64x64 pixels
There are 5 layers in total which each represent different types of data
- "LayerRed":			Layer that is entirely red, we expect the red channel to be entirely white (255) while the rest is 0
- "LayerGreen":			Same as above but entirely green
- "LayerBlue":			Same as above but entirely blue
- "LayerFirstRowRed":	The entire layer is black except for the first row which is red (255, 0, 0). We expect the data to reflect this
- "Layer_R255_G128_B0":	The layer has the R, G and B values indicated in the layer name across the whole document
*/
#pragma once

#include "doctest.h"

#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "Macros.h"

#include <filesystem>


// Implementation of the below function. This specialization is necessary due to the way in which we access the layerinfo for 8, 16 and 32 bit files
template <typename T>
void checkDecompressionFileImpl(NAMESPACE_PSAPI::LayerInfo& layerInformation, const double zero_val, const double val_128, const double one_val, const double red_zero_val);


// function to read the files found in the compression folder with which we can continuously retest the same layer structure as well as easily expand
// on further test case.
// Note the red_zero_val parameter here. This is not a mistake. When saving out a completely red channel in 16-bit mode the other channels will actually be 
// of a value of 2. This is only for completely red pixels and I am unsure why this is the case
template <typename T>
void checkDecompressionFile(std::filesystem::path& inputPath, const double zero_val, const double val_128, const double one_val, const double red_zero_val);
