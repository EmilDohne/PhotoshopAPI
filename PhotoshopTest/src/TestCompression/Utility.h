/*
These functions test a specific type of file which are currently located under PhotoshopTest/documents/Compression/
If these were to change in the future to e.g. expand on the tests or improve upon them the checkCompressionFileImpl() function would need
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
#include "Core/FileIO/Read.h"

#include <filesystem>


template <typename T>
void checkLayerIsSame(NAMESPACE_PSAPI::LayerInfo& layerInformation, const NAMESPACE_PSAPI::FileHeader& header, NAMESPACE_PSAPI::File& document, int layerIndex);

// The implementation of the below function to avoid code duplication
template <typename T>
void checkCompressionFileImpl(NAMESPACE_PSAPI::LayerInfo& layerInformation, const NAMESPACE_PSAPI::FileHeader& header, NAMESPACE_PSAPI::File& document);

// Check if our compression of a input stream matches the compression by photoshop via roundtripping. To do this we read the data -> uncompress -> recompress and compare.
// Calls upon checkCompressionFileImpl to actually do the comparisons, in here we simply redirect the image data based on the bitdepth
template <typename T>
void checkCompressionFile(std::filesystem::path& inputPath);
