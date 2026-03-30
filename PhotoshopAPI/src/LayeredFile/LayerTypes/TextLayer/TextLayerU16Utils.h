#pragma once

#include "Macros.h"
#include "TextLayerTypes.h"
#include "TextLayerParsingUtils.h"
#include "Core/Struct/UnicodeString.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <vector>

PSAPI_NAMESPACE_BEGIN

namespace TextLayerDetail
{

inline std::u16string decode_utf16be_bytes(const std::vector<std::byte>& bytes, bool& has_bom)
{
	has_bom = false;
	if (bytes.size() < 2u)
	{
		return {};
	}

	size_t start = 0u;
	if (to_u8(bytes[0]) == 0xFEu && to_u8(bytes[1]) == 0xFFu)
	{
		has_bom = true;
		start = 2u;
	}

	const auto code_units = (bytes.size() - start) / 2u;
	std::u16string out;
	out.reserve(code_units);
	for (size_t i = 0u; i < code_units; ++i)
	{
		const auto high = static_cast<uint16_t>(to_u8(bytes[start + i * 2u]));
		const auto low = static_cast<uint16_t>(to_u8(bytes[start + i * 2u + 1u]));
		out.push_back(static_cast<char16_t>((high << 8) | low));
	}
	return out;
}

inline std::vector<std::byte> encode_utf16be_bytes(const std::u16string& utf16, const bool has_bom)
{
	const size_t bom_bytes = has_bom ? 2u : 0u;
	std::vector<std::byte> out(utf16.size() * 2u + bom_bytes);
	size_t offset = 0u;
	if (has_bom)
	{
		out[0] = static_cast<std::byte>(0xFEu);
		out[1] = static_cast<std::byte>(0xFFu);
		offset = 2u;
	}

	for (size_t i = 0u; i < utf16.size(); ++i)
	{
		const auto code_unit = static_cast<uint16_t>(utf16[i]);
		out[offset + i * 2u] = static_cast<std::byte>((code_unit >> 8) & 0xFFu);
		out[offset + i * 2u + 1u] = static_cast<std::byte>(code_unit & 0xFFu);
	}
	return out;
}

/// Encode a UTF-8 string into the BOM + UTF-16BE raw representation
/// used by EngineData LiteralString font names.
/// Bytes that are special in PostScript literal-string syntax —
/// '(' (0x28), ')' (0x29), and '\\' (0x5C) — are escaped with a
/// preceding backslash so that the EngineData parser handles them
/// correctly (the parser operates byte-by-byte, not at code-unit level).
inline std::string encode_engine_literal_utf16be(const std::string& utf8)
{
	auto utf16le = UnicodeString::convertUTF8ToUTF16LE(utf8);
	// Build BOM (FE FF) + UTF-16BE bytes, escaping PS-special bytes.
	std::string result;
	result.reserve(2u + utf16le.size() * 2u + 8u); // small headroom for escapes
	result.push_back(static_cast<char>(0xFE));
	result.push_back(static_cast<char>(0xFF));
	for (const char16_t ch : utf16le)
	{
		// High byte (big-endian first)
		const auto hi = static_cast<char>((ch >> 8) & 0xFF);
		const auto lo = static_cast<char>(ch & 0xFF);
		// Escape any byte that is '(', ')', or '\\' in PostScript syntax
		auto push_escaped = [&](char byte)
		{
			const auto u = static_cast<uint8_t>(byte);
			if (u == 0x28u || u == 0x29u || u == 0x5Cu)
			{
				result.push_back('\\');
			}
			result.push_back(byte);
		};
		push_escaped(hi);
		push_escaped(lo);
	}
	return result;
}

/// Decode a UTF-16BE EngineData LiteralString to a UTF-8 std::string.
inline std::string decode_engine_literal_utf16be(const std::string& raw)
{
	// Convert char-based string_value to byte vector for decode_utf16be_bytes
	std::vector<std::byte> bytes;
	bytes.reserve(raw.size());
	for (const auto c : raw)
	{
		bytes.push_back(static_cast<std::byte>(static_cast<uint8_t>(c)));
	}

	bool has_bom = false;
	auto utf16be = decode_utf16be_bytes(bytes, has_bom);
	if (utf16be.empty())
	{
		return {};
	}

	return UnicodeString::convertUTF16LEtoUTF8(utf16be);
}

inline std::optional<size_t> find_substring_u16(const std::u16string& haystack, const std::u16string& needle, const size_t start)
{
	if (needle.empty() || start > haystack.size())
	{
		return std::nullopt;
	}

	const auto it = std::search(haystack.begin() + static_cast<std::ptrdiff_t>(start), haystack.end(), needle.begin(), needle.end());
	if (it == haystack.end())
	{
		return std::nullopt;
	}
	return static_cast<size_t>(std::distance(haystack.begin(), it));
}

inline std::optional<Utf16ReplaceResult> replace_utf16(
	const std::u16string& source,
	const std::u16string& old_text,
	const std::u16string& new_text,
	const bool replace_all)
{
	if (old_text.empty())
	{
		return std::nullopt;
	}

	Utf16ReplaceResult result{};
	result.text_utf16.reserve(source.size());

	size_t copy_pos = 0u;
	size_t search_pos = 0u;
	while (true)
	{
		const auto match_opt = find_substring_u16(source, old_text, search_pos);
		if (!match_opt.has_value())
		{
			break;
		}

		const size_t match = match_opt.value();
		result.text_utf16.append(
			source.begin() + static_cast<std::ptrdiff_t>(copy_pos),
			source.begin() + static_cast<std::ptrdiff_t>(match)
		);
		result.text_utf16.append(new_text);

		result.replacements.push_back(TextReplacement{
			match,
			old_text.size(),
			new_text.size()
			});

		copy_pos = match + old_text.size();
		search_pos = copy_pos;

		if (!replace_all)
		{
			break;
		}
	}

	if (result.replacements.empty())
	{
		return std::nullopt;
	}

	result.text_utf16.append(source.begin() + static_cast<std::ptrdiff_t>(copy_pos), source.end());
	return result;
}

inline size_t remap_plain_index(const size_t old_index, const std::vector<TextReplacement>& replacements)
{
	int64_t delta = 0;

	for (const auto& replacement : replacements)
	{
		const size_t segment_start = replacement.old_start;
		const size_t segment_end = replacement.old_start + replacement.old_length;

		if (old_index < segment_start)
		{
			break;
		}

		if (old_index > segment_end)
		{
			delta += static_cast<int64_t>(replacement.new_length) - static_cast<int64_t>(replacement.old_length);
			continue;
		}

		if (old_index == segment_start)
		{
			return static_cast<size_t>(static_cast<int64_t>(segment_start) + delta);
		}

		if (old_index == segment_end)
		{
			return static_cast<size_t>(static_cast<int64_t>(segment_start) + delta + static_cast<int64_t>(replacement.new_length));
		}

		const auto inside = old_index - segment_start;
		const auto clamped_inside = std::min(inside, replacement.new_length);
		return static_cast<size_t>(static_cast<int64_t>(segment_start) + delta + static_cast<int64_t>(clamped_inside));
	}

	return static_cast<size_t>(static_cast<int64_t>(old_index) + delta);
}

inline size_t remap_engine_index(const size_t old_index, const size_t old_plain_units, const std::vector<TextReplacement>& replacements)
{
	if (old_index <= old_plain_units)
	{
		return remap_plain_index(old_index, replacements);
	}

	const auto mapped_plain_end = remap_plain_index(old_plain_units, replacements);
	return mapped_plain_end + (old_index - old_plain_units);
}

inline std::vector<int32_t> remap_run_lengths(
	const std::vector<int32_t>& old_lengths,
	const size_t old_plain_units,
	const std::vector<TextReplacement>& replacements)
{
	if (old_lengths.empty())
	{
		return {};
	}

	std::vector<size_t> old_boundaries;
	old_boundaries.reserve(old_lengths.size() + 1u);
	old_boundaries.push_back(0u);
	for (const auto length : old_lengths)
	{
		if (length < 0)
		{
			return {};
		}
		old_boundaries.push_back(old_boundaries.back() + static_cast<size_t>(length));
	}

	std::vector<size_t> new_boundaries;
	new_boundaries.reserve(old_boundaries.size());
	for (const auto old_boundary : old_boundaries)
	{
		auto mapped = remap_engine_index(old_boundary, old_plain_units, replacements);
		if (!new_boundaries.empty() && mapped < new_boundaries.back())
		{
			mapped = new_boundaries.back();
		}
		new_boundaries.push_back(mapped);
	}

	std::vector<int32_t> out;
	out.reserve(old_lengths.size());
	for (size_t i = 1u; i < new_boundaries.size(); ++i)
	{
		const auto diff = new_boundaries[i] - new_boundaries[i - 1u];
		if (diff > static_cast<size_t>(std::numeric_limits<int32_t>::max()))
		{
			return {};
		}
		out.push_back(static_cast<int32_t>(diff));
	}
	return out;
}

} // namespace TextLayerDetail

PSAPI_NAMESPACE_END
