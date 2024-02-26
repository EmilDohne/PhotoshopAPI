#pragma once

#include "Macros.h"
#include "Logger.h"

#include <variant>

PSAPI_NAMESPACE_BEGIN


// Rounds a value of type T to the given padding and returns the padded value.
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename T>
inline T RoundUpToMultiple(T value, T padding)
{
	if (value < 0)
	{
		PSAPI_LOG_ERROR("RoundUpToMultiple", "Cannot round up a negative value, returning 0");
		return T{};
	}
	return ((value + padding - 1) / padding) * padding;
}


// Figure out, at runtime, how big a variable is depending on the version specified in the file header
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename TPsd, typename TPsb>
uint32_t SwapPsdPsb(const Enum::Version version)
{
	switch (version)
	{
	case Enum::Version::Psd:
		return sizeof(TPsd);
	case Enum::Version::Psb:
		return sizeof(TPsb);
	default:
		return 0u;
	}
}



// Extract a value from an std::variant and return the PSB type (usually the widest type). Useful when used in conjunction with ReadBinaryDataVariadic
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
template <typename TPsd, typename TPsb>
TPsb ExtractWidestValue(std::variant<TPsd, TPsb> variant)
{
	if constexpr (sizeof(TPsb) < sizeof(TPsd))
	{
		PSAPI_LOG_WARNING("ExtractWidestValue", "PSD value is wider in size than PSB value, will cast down. Might overflow");
	}

	if (std::holds_alternative<TPsd>(variant))
	{
		return static_cast<TPsb>(std::get<TPsd>(variant));
	}
	return std::get<TPsb>(variant);
}




PSAPI_NAMESPACE_END