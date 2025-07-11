#pragma once

#include "Macros.h"
#include "Core/Endian/EndianByteSwap.h"

#include <span>
#include <array>
#include <cstdint>
#include <bit>
#include <stdexcept>
#include <cstring>
#include <type_traits>

PSAPI_NAMESPACE_BEGIN

namespace bytes_io
{

    /// \brief Reads a trivially copyable type T from a byte span using std::bit_cast.
    /// 
    /// This utility reads a value of type T from a span of bytes starting at a given offset.
    /// It ensures that the read does not exceed the bounds of the input span and that T is
    /// trivially copyable. It supports both `uint8_t` and `std::byte` as byte representations.
    /// 
    /// \tparam T The type to read. Must be trivially copyable.
    /// \tparam ByteType The type of the bytes in the span. Must be either `uint8_t` or `std::byte`.
    /// \param file_data The span of input bytes from which to read.
    /// \param offset The offset in the span at which to begin reading.
    /// \return The value of type T read from the span.
    /// \throws std::out_of_range if the offset and size of T exceed the span bounds.
    template <typename T, typename ByteType>
        requires std::is_same_v<ByteType, uint8_t> || std::is_same_v<ByteType, std::byte>
    T read_as(const std::span<const ByteType> file_data, size_t offset) 
    {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

        if (offset + sizeof(T) > file_data.size()) 
        {
            throw std::out_of_range("Offset plus sizeof(T) exceeds data bounds.");
        }

        // Create a buffer of T and bit_cast it
        std::array<ByteType, sizeof(T)> buffer;
        std::memcpy(buffer.data(), file_data.data() + offset, sizeof(T));

        return std::bit_cast<T>(buffer);
    }

    /// \brief Reads a trivially copyable type T from a byte span using std::bit_cast.
    /// 
    /// This utility reads a value of type T from a span of bytes starting at a given offset.
    /// It ensures that the read does not exceed the bounds of the input span and that T is
    /// trivially copyable. It supports both `uint8_t` and `std::byte` as byte representations.
    /// 
    /// \tparam T The type to read. Must be trivially copyable.
    /// \tparam ByteType The type of the bytes in the span. Must be either `uint8_t` or `std::byte`.
    /// \param file_data The span of input bytes from which to read.
    /// \param offset The offset in the span at which to begin reading.
    /// \return The value of type T read from the span.
    /// \throws std::out_of_range if the offset and size of T exceed the span bounds.
    template <typename T, typename ByteType>
        requires std::is_same_v<ByteType, uint8_t> || std::is_same_v<ByteType, std::byte>
    T read_as_and_swap(const std::span<const ByteType> file_data, size_t offset)
    {
        T result = read_as<T, ByteType>(file_data, offset);
        return endian_encode_be<T>(result);
    }

} // bytes_io

PSAPI_NAMESPACE_END