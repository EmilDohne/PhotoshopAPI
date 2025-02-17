#pragma once

#include "Macros.h"
#include "Util/Logger.h"
#include "Util/Profiling/Perf/Instrumentor.h"

#include <concepts>
#include <type_traits>
#include <cstdint>
#include <bit>


PSAPI_NAMESPACE_BEGIN


namespace impl
{
    // Generic function to decode a big-endian value of arbitrary size
    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    template<typename T>
        requires std::is_integral_v<T>
    T _endian_decode_be(const std::byte* src)
    {
		using unsigned_T = std::make_unsigned_t<T>; // Convert to unsigned for correct bitwise operations
		unsigned_T result = 0;

		for (size_t i = 0; i < sizeof(T); ++i)
		{
			result |= static_cast<unsigned_T>(src[i]) << ((sizeof(T) - 1 - i) * 8);
		}

		return static_cast<T>(result);
    }


	// Generic function to encode a big-endian value of arbitrary size
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template<typename T>
		requires std::is_integral_v<T>
	T _endian_encode_be(T value)
	{
		using unsigned_T = std::make_unsigned_t<T>; // Convert to unsigned for bitwise operations
		unsigned_T uvalue = static_cast<unsigned_T>(value);

        T ret = 0;
        std::byte* dest = reinterpret_cast<std::byte*>(&ret);

		for (size_t i = 0; i < sizeof(T); ++i)
		{
			dest[i] = static_cast<std::byte>((uvalue >> ((sizeof(T) - 1 - i) * 8)));
		}

        return ret;
	}
}
	
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
T endian_decode_be(const std::byte* src)
{
    if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>)
    {
        return impl::_endian_decode_be<T>(src);
    }
    else
    {
        using int_T = std::conditional_t<sizeof(T) == 1, uint8_t,
            std::conditional_t < sizeof(T) == 2, uint16_t,
            std::conditional_t < sizeof(T) == 4, uint32_t,
            std::conditional_t < sizeof(T) == 8, uint64_t, void>>>>;
        static_assert(!std::is_void_v<int_T>, "Unsupported type size");

        int_T int_value = impl::_endian_decode_be<int_T>(src);
        return std::bit_cast<T>(int_value);
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
T endian_encode_be(T src)
{
	if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>)
	{
		return impl::_endian_encode_be<T>(src);
	}
	else
	{
		using int_T = std::conditional_t<sizeof(T) == 1, uint8_t,
			std::conditional_t < sizeof(T) == 2, uint16_t,
			std::conditional_t < sizeof(T) == 4, uint32_t,
			std::conditional_t < sizeof(T) == 8, uint64_t, void>>>>;
		static_assert(!std::is_void_v<int_T>, "Unsupported type size");

		int_T int_value = impl::_endian_encode_be<int_T>(std::bit_cast<int_T>(src));
		return std::bit_cast<T>(int_value);
	}
}


PSAPI_NAMESPACE_END