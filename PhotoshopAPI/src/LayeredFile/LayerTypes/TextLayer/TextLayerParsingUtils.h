#pragma once

#include "Macros.h"
#include "TextLayerTypes.h"
#include "Core/Endian/EndianByteSwap.h"
#include "Core/FileIO/BytesIO.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iomanip>
#include <limits>
#include <optional>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

PSAPI_NAMESPACE_BEGIN

namespace TextLayerDetail
{

inline uint8_t to_u8(const std::byte value)
{
	return std::to_integer<uint8_t>(value);
}

inline uint16_t read_u16_be(const std::vector<std::byte>& data, const size_t offset)
{
	return bytes_io::read_as_and_swap<uint16_t, std::byte>(
		std::span<const std::byte>(data.data(), data.size()),
		offset);
}

inline uint32_t read_u32_be(const std::vector<std::byte>& data, const size_t offset)
{
	return bytes_io::read_as_and_swap<uint32_t, std::byte>(
		std::span<const std::byte>(data.data(), data.size()),
		offset);
}

inline int32_t read_i32_be(const std::vector<std::byte>& data, const size_t offset)
{
	return static_cast<int32_t>(read_u32_be(data, offset));
}

inline double read_double_be(const std::vector<std::byte>& data, const size_t offset)
{
	return bytes_io::read_as_and_swap<double, std::byte>(
		std::span<const std::byte>(data.data(), data.size()),
		offset);
}

inline void write_u32_be(std::vector<std::byte>& data, const size_t offset, const uint32_t value)
{
	const uint32_t encoded = endian_encode_be<uint32_t>(value);
	std::memcpy(data.data() + static_cast<std::ptrdiff_t>(offset), &encoded, sizeof(uint32_t));
}

inline void write_i32_be(std::vector<std::byte>& data, const size_t offset, const int32_t value)
{
	write_u32_be(data, offset, static_cast<uint32_t>(value));
}

inline void write_double_be(std::vector<std::byte>& data, const size_t offset, const double value)
{
	const double encoded = endian_encode_be<double>(value);
	std::memcpy(data.data() + static_cast<std::ptrdiff_t>(offset), &encoded, sizeof(double));
}

inline void write_utf16le_as_be(std::vector<std::byte>& data, const size_t byte_offset, const std::u16string& utf16le)
{
	for (size_t i = 0; i < utf16le.size(); ++i)
	{
		const auto code_unit = static_cast<uint16_t>(utf16le[i]);
		data[byte_offset + i * 2u] = static_cast<std::byte>((code_unit >> 8) & 0xFFu);
		data[byte_offset + i * 2u + 1u] = static_cast<std::byte>(code_unit & 0xFFu);
	}
}

inline bool ascii_equal_at(const std::vector<std::byte>& data, const size_t offset, const std::string_view needle)
{
	if (offset + needle.size() > data.size())
	{
		return false;
	}

	for (size_t i = 0u; i < needle.size(); ++i)
	{
		if (to_u8(data[offset + i]) != static_cast<uint8_t>(needle[i]))
		{
			return false;
		}
	}
	return true;
}

inline std::optional<size_t> find_ascii(const std::vector<std::byte>& data, const std::string_view needle, const size_t start = 0u)
{
	if (needle.empty() || start >= data.size() || needle.size() > data.size() - start)
	{
		return std::nullopt;
	}

	for (size_t i = start; i + needle.size() <= data.size(); ++i)
	{
		if (ascii_equal_at(data, i, needle))
		{
			return i;
		}
	}
	return std::nullopt;
}

inline bool is_ascii_whitespace(const uint8_t c)
{
	return std::isspace(static_cast<unsigned char>(c)) != 0;
}

inline size_t skip_ascii_whitespace(const std::vector<std::byte>& data, size_t cursor, const size_t end)
{
	const auto clamped_end = std::min(end, data.size());
	while (cursor < clamped_end && is_ascii_whitespace(to_u8(data[cursor])))
	{
		++cursor;
	}
	return cursor;
}

inline size_t skip_literal_string(const std::vector<std::byte>& data, const size_t start)
{
	if (start >= data.size() || to_u8(data[start]) != static_cast<uint8_t>('('))
	{
		return start;
	}

	bool escape = false;
	int32_t nesting = 1;
	for (size_t i = start + 1u; i < data.size(); ++i)
	{
		const auto c = to_u8(data[i]);
		if (escape)
		{
			escape = false;
			continue;
		}
		if (c == static_cast<uint8_t>('\\'))
		{
			escape = true;
			continue;
		}
		if (c == static_cast<uint8_t>('('))
		{
			++nesting;
			continue;
		}
		if (c == static_cast<uint8_t>(')'))
		{
			--nesting;
			if (nesting == 0)
			{
				return i + 1u;
			}
		}
	}

	return data.size();
}

inline size_t skip_hex_string(const std::vector<std::byte>& data, const size_t start)
{
	if (start >= data.size() || to_u8(data[start]) != static_cast<uint8_t>('<'))
	{
		return start;
	}

	for (size_t i = start + 1u; i < data.size(); ++i)
	{
		if (to_u8(data[i]) == static_cast<uint8_t>('>'))
		{
			return i + 1u;
		}
	}

	return data.size();
}

inline std::optional<size_t> find_matching_dictionary_end(const std::vector<std::byte>& data, const size_t dict_open)
{
	if (dict_open + 1u >= data.size() || !ascii_equal_at(data, dict_open, "<<"))
	{
		return std::nullopt;
	}

	size_t depth = 1u;
	size_t cursor = dict_open + 2u;
	while (cursor < data.size())
	{
		const auto c = to_u8(data[cursor]);
		if (c == static_cast<uint8_t>('('))
		{
			cursor = skip_literal_string(data, cursor);
			continue;
		}
		if (c == static_cast<uint8_t>('<') &&
			(cursor + 1u >= data.size() || to_u8(data[cursor + 1u]) != static_cast<uint8_t>('<')))
		{
			cursor = skip_hex_string(data, cursor);
			continue;
		}

		if (cursor + 1u < data.size() && ascii_equal_at(data, cursor, "<<"))
		{
			++depth;
			cursor += 2u;
			continue;
		}
		if (cursor + 1u < data.size() && ascii_equal_at(data, cursor, ">>"))
		{
			--depth;
			cursor += 2u;
			if (depth == 0u)
			{
				return cursor;
			}
			continue;
		}
		++cursor;
	}

	return std::nullopt;
}

inline std::optional<size_t> find_matching_array_end(const std::vector<std::byte>& data, const size_t array_open)
{
	if (array_open >= data.size() || to_u8(data[array_open]) != static_cast<uint8_t>('['))
	{
		return std::nullopt;
	}

	size_t depth = 1u;
	size_t cursor = array_open + 1u;
	while (cursor < data.size())
	{
		const auto c = to_u8(data[cursor]);
		if (c == static_cast<uint8_t>('('))
		{
			cursor = skip_literal_string(data, cursor);
			continue;
		}
		if (c == static_cast<uint8_t>('<') &&
			(cursor + 1u >= data.size() || to_u8(data[cursor + 1u]) != static_cast<uint8_t>('<')))
		{
			cursor = skip_hex_string(data, cursor);
			continue;
		}

		if (c == static_cast<uint8_t>('['))
		{
			++depth;
			++cursor;
			continue;
		}
		if (c == static_cast<uint8_t>(']'))
		{
			--depth;
			++cursor;
			if (depth == 0u)
			{
				return cursor;
			}
			continue;
		}
		++cursor;
	}

	return std::nullopt;
}

inline std::optional<StyleRunInfo> parse_style_run_info(const std::vector<std::byte>& payload)
{
	size_t search_pos = 0u;
	while (true)
	{
		const auto style_pos_opt = find_ascii(payload, "/StyleRun", search_pos);
		if (!style_pos_opt.has_value())
		{
			return std::nullopt;
		}

		size_t style_dict_start = skip_ascii_whitespace(
			payload,
			style_pos_opt.value() + std::string_view("/StyleRun").size(),
			payload.size()
		);
		if (style_dict_start + 1u >= payload.size() || !ascii_equal_at(payload, style_dict_start, "<<"))
		{
			search_pos = style_pos_opt.value() + 1u;
			continue;
		}

		const auto style_dict_end_opt = find_matching_dictionary_end(payload, style_dict_start);
		if (!style_dict_end_opt.has_value())
		{
			search_pos = style_pos_opt.value() + 1u;
			continue;
		}

		const auto run_array_pos_opt = find_ascii(payload, "/RunArray", style_dict_start);
		if (!run_array_pos_opt.has_value() || run_array_pos_opt.value() >= style_dict_end_opt.value())
		{
			search_pos = style_pos_opt.value() + 1u;
			continue;
		}

		size_t run_array_open = skip_ascii_whitespace(
			payload,
			run_array_pos_opt.value() + std::string_view("/RunArray").size(),
			style_dict_end_opt.value()
		);
		if (run_array_open >= payload.size() || to_u8(payload[run_array_open]) != static_cast<uint8_t>('['))
		{
			search_pos = style_pos_opt.value() + 1u;
			continue;
		}

		const auto run_array_end_opt = find_matching_array_end(payload, run_array_open);
		if (!run_array_end_opt.has_value() || run_array_end_opt.value() > style_dict_end_opt.value())
		{
			search_pos = style_pos_opt.value() + 1u;
			continue;
		}

		StyleRunInfo info{};
		info.run_array_open = run_array_open;
		info.run_array_end = run_array_end_opt.value();

		size_t cursor = run_array_open + 1u;
		while (cursor < run_array_end_opt.value() - 1u)
		{
			cursor = skip_ascii_whitespace(payload, cursor, run_array_end_opt.value() - 1u);
			if (cursor + 1u >= run_array_end_opt.value() - 1u)
			{
				break;
			}

			if (ascii_equal_at(payload, cursor, "<<"))
			{
				const auto dict_end = find_matching_dictionary_end(payload, cursor);
				if (!dict_end.has_value() || dict_end.value() > run_array_end_opt.value() - 1u)
				{
					return std::nullopt;
				}

				info.runs.push_back(StyleRunSpan{ cursor, dict_end.value() });
				cursor = dict_end.value();
				continue;
			}

			++cursor;
		}

		return info;
	}
}

inline std::optional<ByteRange> parse_number_token(const std::vector<std::byte>& data, size_t cursor, const size_t end)
{
	const auto clamped_end = std::min(end, data.size());
	if (cursor >= clamped_end)
	{
		return std::nullopt;
	}

	const size_t token_start = cursor;
	auto c = to_u8(data[cursor]);
	if (c == static_cast<uint8_t>('+') || c == static_cast<uint8_t>('-'))
	{
		++cursor;
	}

	bool has_digits = false;
	while (cursor < clamped_end && std::isdigit(static_cast<unsigned char>(to_u8(data[cursor]))))
	{
		has_digits = true;
		++cursor;
	}

	if (cursor < clamped_end && to_u8(data[cursor]) == static_cast<uint8_t>('.'))
	{
		++cursor;
		while (cursor < clamped_end && std::isdigit(static_cast<unsigned char>(to_u8(data[cursor]))))
		{
			has_digits = true;
			++cursor;
		}
	}

	if (!has_digits)
	{
		return std::nullopt;
	}

	if (cursor < clamped_end)
	{
		c = to_u8(data[cursor]);
		if (c == static_cast<uint8_t>('e') || c == static_cast<uint8_t>('E'))
		{
			size_t exponent_cursor = cursor + 1u;
			if (exponent_cursor < clamped_end)
			{
				const auto exp_sign = to_u8(data[exponent_cursor]);
				if (exp_sign == static_cast<uint8_t>('+') || exp_sign == static_cast<uint8_t>('-'))
				{
					++exponent_cursor;
				}
			}

			size_t exponent_digits = exponent_cursor;
			while (exponent_cursor < clamped_end && std::isdigit(static_cast<unsigned char>(to_u8(data[exponent_cursor]))))
			{
				++exponent_cursor;
			}

			if (exponent_digits != exponent_cursor)
			{
				cursor = exponent_cursor;
			}
		}
	}

	return ByteRange{ token_start, cursor };
}

inline std::optional<ByteRange> find_number_value_after_key(
	const std::vector<std::byte>& data,
	const size_t range_start,
	const size_t range_end,
	const std::string_view key)
{
	size_t search_pos = range_start;
	while (search_pos < range_end)
	{
		const auto key_pos = find_ascii(data, key, search_pos);
		if (!key_pos.has_value() || key_pos.value() >= range_end)
		{
			break;
		}

		size_t value_start = skip_ascii_whitespace(data, key_pos.value() + key.size(), range_end);
		const auto number_span = parse_number_token(data, value_start, range_end);
		if (number_span.has_value())
		{
			return number_span;
		}

		search_pos = key_pos.value() + 1u;
	}

	return std::nullopt;
}

inline std::optional<double> parse_double_from_range(const std::vector<std::byte>& data, const ByteRange range)
{
	if (range.end <= range.start || range.end > data.size())
	{
		return std::nullopt;
	}

	std::string token;
	token.reserve(range.end - range.start);
	for (size_t i = range.start; i < range.end; ++i)
	{
		token.push_back(static_cast<char>(to_u8(data[i])));
	}

	try
	{
		return std::stod(token);
	}
	catch (...)
	{
		return std::nullopt;
	}
}

inline std::vector<double> parse_number_list(const std::vector<std::byte>& data, const size_t range_start, const size_t range_end)
{
	std::vector<double> out;
	size_t cursor = range_start;
	while (cursor < range_end)
	{
		cursor = skip_ascii_whitespace(data, cursor, range_end);
		if (cursor >= range_end)
		{
			break;
		}

		const auto number_span = parse_number_token(data, cursor, range_end);
		if (!number_span.has_value())
		{
			++cursor;
			continue;
		}

		const auto parsed = parse_double_from_range(data, number_span.value());
		if (parsed.has_value())
		{
			out.push_back(parsed.value());
		}
		cursor = number_span->end;
	}
	return out;
}

inline std::optional<std::string> format_number_token(const double value)
{
	if (!std::isfinite(value))
	{
		return std::nullopt;
	}

	std::ostringstream stream;
	stream.setf(std::ios::fixed);
	stream << std::setprecision(6) << value;

	std::string out = stream.str();
	while (out.size() > 2u && out.back() == '0')
	{
		out.pop_back();
	}
	if (!out.empty() && out.back() == '.')
	{
		out.push_back('0');
	}
	if (out == "-0.0")
	{
		out = "0.0";
	}
	return out;
}

inline std::optional<std::string> format_numeric_array(const std::vector<double>& values)
{
	if (values.empty())
	{
		return std::nullopt;
	}

	std::string out = " ";
	for (size_t i = 0u; i < values.size(); ++i)
	{
		const auto token = format_number_token(values[i]);
		if (!token.has_value())
		{
			return std::nullopt;
		}

		if (i > 0u)
		{
			out += " ";
		}
		out += token.value();
	}
	out += " ";
	return out;
}

inline void replace_range(
	std::vector<std::byte>& data,
	const size_t start,
	const size_t end,
	const std::vector<std::byte>& replacement)
{
	data.erase(
		data.begin() + static_cast<std::ptrdiff_t>(start),
		data.begin() + static_cast<std::ptrdiff_t>(end)
	);
	data.insert(
		data.begin() + static_cast<std::ptrdiff_t>(start),
		replacement.begin(),
		replacement.end()
	);
}

inline std::vector<std::byte> ascii_bytes(const std::string& text)
{
	std::vector<std::byte> bytes;
	bytes.reserve(text.size());
	for (const auto c : text)
	{
		bytes.push_back(static_cast<std::byte>(static_cast<uint8_t>(c)));
	}
	return bytes;
}

inline std::optional<ByteRange> find_fill_color_values_content_range(
	const std::vector<std::byte>& payload,
	const StyleRunSpan& run)
{
	size_t search_pos = run.dict_start;
	while (search_pos < run.dict_end)
	{
		const auto fill_color_pos = find_ascii(payload, "/FillColor", search_pos);
		if (!fill_color_pos.has_value() || fill_color_pos.value() >= run.dict_end)
		{
			return std::nullopt;
		}

		size_t fill_dict_open = skip_ascii_whitespace(
			payload,
			fill_color_pos.value() + std::string_view("/FillColor").size(),
			run.dict_end
		);
		if (fill_dict_open + 1u >= run.dict_end || !ascii_equal_at(payload, fill_dict_open, "<<"))
		{
			search_pos = fill_color_pos.value() + 1u;
			continue;
		}

		const auto fill_dict_end = find_matching_dictionary_end(payload, fill_dict_open);
		if (!fill_dict_end.has_value() || fill_dict_end.value() > run.dict_end)
		{
			return std::nullopt;
		}

		const auto values_pos = find_ascii(payload, "/Values", fill_dict_open);
		if (!values_pos.has_value() || values_pos.value() >= fill_dict_end.value())
		{
			return std::nullopt;
		}

		size_t values_open = skip_ascii_whitespace(
			payload,
			values_pos.value() + std::string_view("/Values").size(),
			fill_dict_end.value()
		);
		if (values_open >= payload.size() || to_u8(payload[values_open]) != static_cast<uint8_t>('['))
		{
			return std::nullopt;
		}

		const auto values_end = find_matching_array_end(payload, values_open);
		if (!values_end.has_value() || values_end.value() > fill_dict_end.value())
		{
			return std::nullopt;
		}

		return ByteRange{ values_open + 1u, values_end.value() - 1u };
	}

	return std::nullopt;
}

} // namespace TextLayerDetail

PSAPI_NAMESPACE_END
