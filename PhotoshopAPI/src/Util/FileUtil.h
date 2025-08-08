#pragma once


// If we compile with C++<20 we replace the stdlib implementation with the compatibility
// library
#if (__cplusplus < 202002L)
#include "tcb_span.hpp"
#else
#include <span>
#endif

#include <vector>


// Utility functions for converting trivially copyable items or vectors of trivially copyable items into spans
namespace Util
{
	template <typename T>
	std::span<uint8_t> toWritableBytes(T& value)
	{
		static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
		std::span<uint8_t> data(reinterpret_cast<uint8_t*>(&value), sizeof(T));
		return data;
	}

	template <typename T>
	std::span<const uint8_t> toBytes(const T& value)
	{
		static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
		std::span<const uint8_t> data(reinterpret_cast<const uint8_t*>(&value), sizeof(T));
		return data;
	}


	template <typename T>
	std::span<uint8_t> toWritableBytes(std::vector<T>& vec)
	{
		// Ensure the vector is not empty and the type is trivially copyable
		static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
		return std::span<uint8_t>(reinterpret_cast<uint8_t*>(vec.data()), vec.size() * sizeof(T));
	}

	template <typename T>
	std::span<const uint8_t> toBytes(const std::vector<T>& vec)
	{
		// Ensure the vector is not empty and the type is trivially copyable
		static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
		return std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(vec.data()), vec.size() * sizeof(T));
	}

	template <typename T>
	std::span<uint8_t> toWritableBytes(std::span<T> span)
	{
		// Ensure the vector is not empty and the type is trivially copyable
		static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
		return std::span<uint8_t>(reinterpret_cast<uint8_t*>(span.data()), span.size() * sizeof(T));
	}

	template <typename T>
	std::span<const uint8_t> toBytes(const std::span<const T> span)
	{
		// Ensure the vector is not empty and the type is trivially copyable
		static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
		return std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(span.data()), span.size() * sizeof(T));
	}

}