#pragma once

#include <limits> 
#include <climits>
#include <cstdint>

// Configure the Namespace for the PhotoshopAPI here to avoid namespace clashing
#define NAMESPACE_PSAPI PhotoshopAPI
#define PSAPI_NAMESPACE_BEGIN namespace NAMESPACE_PSAPI {
#define PSAPI_NAMESPACE_END }

#define PSAPI_UNUSED(x) (void)(x);

#define PSAPI_PROFILING 1

// Check that float and double are 32 and 64 bit wide respectively 
static_assert(sizeof(float) == 4 && CHAR_BIT == 8 && std::numeric_limits<float>::is_iec559, "float type is not 32 bit wide, this is not currently supported");
static_assert(sizeof(double) == 8 && CHAR_BIT == 8 && std::numeric_limits<double>::is_iec559, "double type is not 64 bit wide, this is not currently supported");

// Alias these types for consistency throughout the code
typedef double float64_t;
typedef float float32_t;

// Alias bitDepth types which can be used for template initialization
PSAPI_NAMESPACE_BEGIN

/// Equivalent of a 1 byte unsigned char.
typedef uint8_t bpp8_t;

/// Equivalent of a 2 byte unsigned char.
typedef uint16_t bpp16_t;

/// Equivalent of a 4 byte IEEE754 floating point number.
typedef float32_t bpp32_t;

PSAPI_NAMESPACE_END