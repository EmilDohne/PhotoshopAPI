#pragma once

#include "Macros.h"

#include <type_traits>

PSAPI_NAMESPACE_BEGIN

namespace concepts
{
	template <typename T>
	concept bit_depth = std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, float>;
}

PSAPI_NAMESPACE_END