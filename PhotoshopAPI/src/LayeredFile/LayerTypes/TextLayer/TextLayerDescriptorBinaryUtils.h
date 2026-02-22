#pragma once

// =========================================================================
//  TextLayerDescriptorBinaryUtils.h
//
//  Low-level binary descriptor serialization helpers used by text layer
//  builder utilities.
// =========================================================================

#include "Macros.h"
#include "Core/Endian/EndianByteSwap.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

PSAPI_NAMESPACE_BEGIN

namespace TextLayerDetail
{
// -----------------------------------------------------------------------
//  Binary descriptor helpers (write Photoshop binary descriptor format)
// -----------------------------------------------------------------------

template <typename T>
inline void push_be_encoded(std::vector<std::byte>& buf, T value)
{
	const T encoded = endian_encode_be<T>(value);
	const auto* bytes = reinterpret_cast<const std::byte*>(&encoded);
	buf.insert(buf.end(), bytes, bytes + sizeof(T));
}

inline void push_u16_be(std::vector<std::byte>& buf, uint16_t v)
{
	push_be_encoded<uint16_t>(buf, v);
}

inline void push_u32_be(std::vector<std::byte>& buf, uint32_t v)
{
	push_be_encoded<uint32_t>(buf, v);
}

inline void push_i32_be(std::vector<std::byte>& buf, int32_t v)
{
	push_be_encoded<int32_t>(buf, v);
}

inline void push_double_be(std::vector<std::byte>& buf, double v)
{
	push_be_encoded<double>(buf, v);
}

inline void push_ascii(std::vector<std::byte>& buf, const std::string& s)
{
	for (char c : s)
	{
		buf.push_back(static_cast<std::byte>(static_cast<uint8_t>(c)));
	}
}

// Write a Photoshop descriptor key (length-prefixed, 0-length means 4-byte charID)
inline void push_descriptor_key(std::vector<std::byte>& buf, const std::string& key)
{
	if (key.size() == 4)
	{
		// Use charID shorthand (length=0 means "4 bytes follow")
		push_u32_be(buf, 0u);
		push_ascii(buf, key);
	}
	else
	{
		push_u32_be(buf, static_cast<uint32_t>(key.size()));
		push_ascii(buf, key);
	}
}

// Write UTF-16BE text descriptor value: "TEXT" + u32 count + UTF-16BE code units
inline void push_text_value(std::vector<std::byte>& buf, const std::u16string& utf16)
{
	push_ascii(buf, "TEXT");
	// code unit count includes trailing null
	const uint32_t count = static_cast<uint32_t>(utf16.size() + 1u);
	push_u32_be(buf, count);
	// Write UTF-16BE
	for (char16_t ch : utf16)
	{
		push_u16_be(buf, static_cast<uint16_t>(ch));
	}
	// trailing null
	push_u16_be(buf, 0u);
}

// Write "tdta" + u32 length + raw bytes
inline void push_tdta_value(std::vector<std::byte>& buf, const std::vector<std::byte>& data)
{
	push_ascii(buf, "tdta");
	push_u32_be(buf, static_cast<uint32_t>(data.size()));
	buf.insert(buf.end(), data.begin(), data.end());
}

// Write "long" + i32
inline void push_long_value(std::vector<std::byte>& buf, int32_t v)
{
	push_ascii(buf, "long");
	push_i32_be(buf, v);
}

// Write "enum" + typeID + value
inline void push_enum_value(std::vector<std::byte>& buf, const std::string& type_id, const std::string& value_id)
{
	push_ascii(buf, "enum");
	push_descriptor_key(buf, type_id);
	push_descriptor_key(buf, value_id);
}

// Write "Objc" descriptor body
inline void push_descriptor_start(std::vector<std::byte>& buf, const std::string& class_name, const std::string& class_id, uint32_t key_count)
{
	push_ascii(buf, "Objc");
	// Unicode class name: Photoshop always writes length=1 with a null char for
	// both top-level and nested descriptors when no real name is needed.
	if (class_name.empty())
	{
		push_u32_be(buf, 1u);   // 1 character
		push_u16_be(buf, 0u);   // null UTF-16 char
	}
	else
	{
		push_u32_be(buf, static_cast<uint32_t>(class_name.size()));
		for (char c : class_name)
		{
			push_u16_be(buf, static_cast<uint16_t>(static_cast<uint8_t>(c)));
		}
	}
	// classID: use explicit length for non-4-byte IDs
	if (class_id.size() == 4)
	{
		push_u32_be(buf, 0u);
		push_ascii(buf, class_id);
	}
	else
	{
		push_u32_be(buf, static_cast<uint32_t>(class_id.size()));
		push_ascii(buf, class_id);
	}
	// key count
	push_u32_be(buf, key_count);
}

// Write "doub" + double
inline void push_doub_value(std::vector<std::byte>& buf, double v)
{
	push_ascii(buf, "doub");
	push_double_be(buf, v);
}

// Write "bool" + 1 byte
inline void push_bool_value(std::vector<std::byte>& buf, bool v)
{
	push_ascii(buf, "bool");
	buf.push_back(static_cast<std::byte>(v ? 1u : 0u));
}

// Write "UntF" + 4-byte unit + 8-byte double
inline void push_untf_value(std::vector<std::byte>& buf, const char* unit, double v)
{
	push_ascii(buf, "UntF");
	push_ascii(buf, std::string(unit, 4));
	push_double_be(buf, v);
}

// Write class name for top-level descriptors: length=1, null char
inline void push_class_name_default(std::vector<std::byte>& buf)
{
	push_u32_be(buf, 1u);      // 1 character
	push_u16_be(buf, 0u);      // null UTF-16 character
}

} // namespace TextLayerDetail

PSAPI_NAMESPACE_END

