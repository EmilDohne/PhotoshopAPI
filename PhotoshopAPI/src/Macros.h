#pragma once

#include<limits> 
#include<climits> 

// Configure the Namespace for the PhotoshopAPI here to avoid namespace clashing
#define NAMESPACE_PSAPI PhotoshopAPI
#define PSAPI_NAMESPACE_BEGIN namespace NAMESPACE_PSAPI {
#define PSAPI_NAMESPACE_END }


// Check that float and double are 32 and 64 bit wide respectively 
static_assert(sizeof(float) == 4 && CHAR_BIT == 8 && std::numeric_limits<float>::is_iec559, "float type is not 32 bit wide, this is not currently supported");
static_assert(sizeof(double) == 8 && CHAR_BIT == 8 && std::numeric_limits<double>::is_iec559, "double type is not 64 bit wide, this is not currently supported");

// Alias these types for consistency throughout the code
typedef double float64_t;
typedef float float32_t;