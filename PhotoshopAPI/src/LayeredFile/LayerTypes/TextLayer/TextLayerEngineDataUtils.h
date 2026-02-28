#pragma once

#include "Macros.h"
#include "TextLayerTypes.h"
#include "TextLayerDescriptorUtils.h"
#include "TextLayerU16Utils.h"

#include "Core/Struct/EngineDataStructure.h"
#include "Core/Struct/UnicodeString.h"
#include "PhotoshopFile/AdditionalLayerInfo.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

PSAPI_NAMESPACE_BEGIN

namespace TextLayerDetail
{

// -----------------------------------------------------------------------
//  Number conversion helpers
// -----------------------------------------------------------------------

inline bool number_value_to_int32(const EngineData::Value& value, int32_t& out)
{
	if (value.type != EngineData::ValueType::Number || !std::isfinite(value.number_value))
	{
		return false;
	}

	const auto rounded = std::round(value.number_value);
	if (std::fabs(rounded - value.number_value) > 0.000001 ||
		rounded < static_cast<double>(std::numeric_limits<int32_t>::min()) ||
		rounded > static_cast<double>(std::numeric_limits<int32_t>::max()))
	{
		return false;
	}

	out = static_cast<int32_t>(rounded);
	return true;
}

// -----------------------------------------------------------------------
//  Font property helpers (operate on EngineData tree)
// -----------------------------------------------------------------------

inline std::optional<std::string> font_property_string(
	const EngineData::Value& root, const size_t font_index, const std::string_view key)
{
	const auto font_set = EngineData::find_by_path(root, { "ResourceDict", "FontSet" });
	if (font_set == nullptr || font_set->type != EngineData::ValueType::Array || font_index >= font_set->array_items.size())
	{
		return std::nullopt;
	}

	const auto& entry = font_set->array_items[font_index];
	const auto value = EngineData::find_dict_value(entry, key);
	if (value == nullptr || value->type != EngineData::ValueType::LiteralString)
	{
		return std::nullopt;
	}

	return decode_engine_literal_utf16be(value->string_value);
}

inline std::optional<int32_t> font_property_int32(
	const EngineData::Value& root, const size_t font_index, const std::string_view key)
{
	const auto font_set = EngineData::find_by_path(root, { "ResourceDict", "FontSet" });
	if (font_set == nullptr || font_set->type != EngineData::ValueType::Array || font_index >= font_set->array_items.size())
	{
		return std::nullopt;
	}

	const auto& entry = font_set->array_items[font_index];
	const auto value = EngineData::find_dict_value(entry, key);
	if (value == nullptr)
	{
		return std::nullopt;
	}

	int32_t result = 0;
	if (!number_value_to_int32(*value, result))
	{
		return std::nullopt;
	}
	return result;
}

// -----------------------------------------------------------------------
//  EngineData tree navigation
// -----------------------------------------------------------------------

inline const EngineData::Value* paragraph_run_properties_for_run(const EngineData::Value& root, const size_t run_index)
{
	const auto run_array = EngineData::find_by_path(root, { "EngineDict", "ParagraphRun", "RunArray" });
	if (run_array == nullptr || run_array->type != EngineData::ValueType::Array || run_index >= run_array->array_items.size())
	{
		return nullptr;
	}

	return EngineData::find_by_path(run_array->array_items[run_index], { "ParagraphSheet", "Properties" });
}

inline EngineData::Value* paragraph_run_properties_for_run(EngineData::Value& root, const size_t run_index)
{
	auto run_array = EngineData::find_by_path(root, { "EngineDict", "ParagraphRun", "RunArray" });
	if (run_array == nullptr || run_array->type != EngineData::ValueType::Array || run_index >= run_array->array_items.size())
	{
		return nullptr;
	}

	return EngineData::find_by_path(run_array->array_items[run_index], { "ParagraphSheet", "Properties" });
}

inline const EngineData::Value* style_run_style_data_for_run(const EngineData::Value& root, const size_t run_index)
{
	const auto run_array = EngineData::find_by_path(root, { "EngineDict", "StyleRun", "RunArray" });
	if (run_array == nullptr || run_array->type != EngineData::ValueType::Array || run_index >= run_array->array_items.size())
	{
		return nullptr;
	}

	return EngineData::find_by_path(run_array->array_items[run_index], { "StyleSheet", "StyleSheetData" });
}

inline EngineData::Value* style_run_style_data_for_run(EngineData::Value& root, const size_t run_index)
{
	auto run_array = EngineData::find_by_path(root, { "EngineDict", "StyleRun", "RunArray" });
	if (run_array == nullptr || run_array->type != EngineData::ValueType::Array || run_index >= run_array->array_items.size())
	{
		return nullptr;
	}

	return EngineData::find_by_path(run_array->array_items[run_index], { "StyleSheet", "StyleSheetData" });
}

inline const EngineData::Value* paragraph_sheet_set_from_root(const EngineData::Value& root)
{
	return EngineData::find_by_path(root, { "ResourceDict", "ParagraphSheetSet" });
}

inline EngineData::Value* paragraph_sheet_set_from_root(EngineData::Value& root)
{
	return EngineData::find_by_path(root, { "ResourceDict", "ParagraphSheetSet" });
}

inline const EngineData::Value* style_sheet_set_from_root(const EngineData::Value& root)
{
	return EngineData::find_by_path(root, { "ResourceDict", "StyleSheetSet" });
}

inline EngineData::Value* style_sheet_set_from_root(EngineData::Value& root)
{
	return EngineData::find_by_path(root, { "ResourceDict", "StyleSheetSet" });
}

inline bool style_normal_sheet_index_from_root(const EngineData::Value& root, int32_t& out_index)
{
	const auto normal_index = EngineData::find_by_path(root, { "ResourceDict", "TheNormalStyleSheet" });
	if (normal_index == nullptr || !number_value_to_int32(*normal_index, out_index))
	{
		return false;
	}

	const auto sheet_set = style_sheet_set_from_root(root);
	if (sheet_set == nullptr || sheet_set->type != EngineData::ValueType::Array || out_index < 0)
	{
		return false;
	}

	return static_cast<size_t>(out_index) < sheet_set->array_items.size();
}

inline const EngineData::Value* style_sheet_data_for_index(const EngineData::Value& root, const size_t sheet_index)
{
	const auto sheet_set = style_sheet_set_from_root(root);
	if (sheet_set == nullptr || sheet_set->type != EngineData::ValueType::Array || sheet_index >= sheet_set->array_items.size())
	{
		return nullptr;
	}

	return EngineData::find_dict_value(sheet_set->array_items[sheet_index], "StyleSheetData");
}

inline EngineData::Value* style_sheet_data_for_index(EngineData::Value& root, const size_t sheet_index)
{
	auto sheet_set = style_sheet_set_from_root(root);
	if (sheet_set == nullptr || sheet_set->type != EngineData::ValueType::Array || sheet_index >= sheet_set->array_items.size())
	{
		return nullptr;
	}

	return EngineData::find_dict_value(sheet_set->array_items[sheet_index], "StyleSheetData");
}

inline const EngineData::Value* style_normal_sheet_data(const EngineData::Value& root)
{
	int32_t normal_index = 0;
	if (!style_normal_sheet_index_from_root(root, normal_index))
	{
		return nullptr;
	}

	return style_sheet_data_for_index(root, static_cast<size_t>(normal_index));
}

inline EngineData::Value* style_normal_sheet_data(EngineData::Value& root)
{
	int32_t normal_index = 0;
	if (!style_normal_sheet_index_from_root(root, normal_index))
	{
		return nullptr;
	}

	return style_sheet_data_for_index(root, static_cast<size_t>(normal_index));
}

inline bool paragraph_normal_sheet_index_from_root(const EngineData::Value& root, int32_t& out_index)
{
	const auto normal_index = EngineData::find_by_path(root, { "ResourceDict", "TheNormalParagraphSheet" });
	if (normal_index == nullptr || !number_value_to_int32(*normal_index, out_index))
	{
		return false;
	}

	const auto sheet_set = paragraph_sheet_set_from_root(root);
	if (sheet_set == nullptr || sheet_set->type != EngineData::ValueType::Array || out_index < 0)
	{
		return false;
	}

	return static_cast<size_t>(out_index) < sheet_set->array_items.size();
}

inline const EngineData::Value* paragraph_sheet_properties_for_index(const EngineData::Value& root, const size_t sheet_index)
{
	const auto sheet_set = paragraph_sheet_set_from_root(root);
	if (sheet_set == nullptr || sheet_set->type != EngineData::ValueType::Array || sheet_index >= sheet_set->array_items.size())
	{
		return nullptr;
	}

	return EngineData::find_dict_value(sheet_set->array_items[sheet_index], "Properties");
}

inline EngineData::Value* paragraph_sheet_properties_for_index(EngineData::Value& root, const size_t sheet_index)
{
	auto sheet_set = paragraph_sheet_set_from_root(root);
	if (sheet_set == nullptr || sheet_set->type != EngineData::ValueType::Array || sheet_index >= sheet_set->array_items.size())
	{
		return nullptr;
	}

	return EngineData::find_dict_value(sheet_set->array_items[sheet_index], "Properties");
}

inline const EngineData::Value* paragraph_normal_sheet_properties(const EngineData::Value& root)
{
	int32_t normal_index = 0;
	if (!paragraph_normal_sheet_index_from_root(root, normal_index))
	{
		return nullptr;
	}

	return paragraph_sheet_properties_for_index(root, static_cast<size_t>(normal_index));
}

inline EngineData::Value* paragraph_normal_sheet_properties(EngineData::Value& root)
{
	int32_t normal_index = 0;
	if (!paragraph_normal_sheet_index_from_root(root, normal_index))
	{
		return nullptr;
	}

	return paragraph_sheet_properties_for_index(root, static_cast<size_t>(normal_index));
}

// -----------------------------------------------------------------------
//  TaggedBlock-level helpers (find/read/write EngineData)
// -----------------------------------------------------------------------

inline std::optional<RawDataSpan> find_engine_data_span(const TaggedBlock& block)
{
	static constexpr std::string_view key = "EngineData";
	static constexpr std::string_view type = "tdta";

	for (size_t i = 0u; i + key.size() + type.size() + sizeof(uint32_t) <= block.m_Data.size(); ++i)
	{
		if (!ascii_equal_at(block.m_Data, i, key))
		{
			continue;
		}

		const size_t type_offset = i + key.size();
		if (!ascii_equal_at(block.m_Data, type_offset, type))
		{
			continue;
		}

		const size_t length_offset = type_offset + type.size();
		const size_t byte_count = static_cast<size_t>(read_u32_be(block.m_Data, length_offset));
		const size_t value_offset = length_offset + sizeof(uint32_t);
		if (value_offset + byte_count > block.m_Data.size())
		{
			continue;
		}

		return RawDataSpan{ length_offset, value_offset, byte_count };
	}

	return std::nullopt;
}

inline std::optional<std::vector<std::byte>> read_engine_payload(const TaggedBlock& block)
{
	const auto engine_span_opt = find_engine_data_span(block);
	if (!engine_span_opt.has_value())
	{
		return std::nullopt;
	}

	return std::vector<std::byte>(
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset),
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset + engine_span_opt->byte_count)
	);
}

inline bool write_engine_payload(TaggedBlock& block, const RawDataSpan& engine_span, const std::vector<std::byte>& payload)
{
	const auto old_payload_size = engine_span.byte_count;
	const auto new_payload_size = payload.size();
	if (new_payload_size > static_cast<size_t>(std::numeric_limits<uint32_t>::max()))
	{
		PSAPI_LOG_ERROR("TextLayer", "EngineData payload exceeded uint32_t length limit");
		return false;
	}

	if (new_payload_size > old_payload_size)
	{
		block.m_Data.insert(
			block.m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span.value_offset + old_payload_size),
			new_payload_size - old_payload_size,
			static_cast<std::byte>(0)
		);
	}
	else if (new_payload_size < old_payload_size)
	{
		block.m_Data.erase(
			block.m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span.value_offset + new_payload_size),
			block.m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span.value_offset + old_payload_size)
		);
	}

	write_u32_be(block.m_Data, engine_span.length_offset, static_cast<uint32_t>(new_payload_size));
	std::copy(
		payload.begin(),
		payload.end(),
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span.value_offset)
	);
	refresh_type_tool_descriptor_cache(block);
	return true;
}

inline std::optional<ByteRange> find_engine_editor_text_value(const std::vector<std::byte>& payload)
{
	const auto editor_pos = find_ascii(payload, "/Editor");
	const size_t text_search_start = editor_pos.has_value() ? editor_pos.value() : 0u;
	const auto text_pos = find_ascii(payload, "/Text (", text_search_start);
	if (!text_pos.has_value())
	{
		return std::nullopt;
	}

	const size_t value_start = text_pos.value() + std::string_view("/Text (").size();
	bool escape = false;
	for (size_t i = value_start; i < payload.size(); ++i)
	{
		const auto c = to_u8(payload[i]);
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

		if (c == static_cast<uint8_t>(')'))
		{
			return ByteRange{ value_start, i };
		}
	}

	return std::nullopt;
}

inline std::vector<RunLengthArraySpan> parse_run_length_arrays(const std::vector<std::byte>& payload)
{
	std::vector<RunLengthArraySpan> arrays;
	size_t search_pos = 0u;

	while (true)
	{
		const auto marker_pos_opt = find_ascii(payload, "/RunLengthArray", search_pos);
		if (!marker_pos_opt.has_value())
		{
			break;
		}

		size_t cursor = marker_pos_opt.value() + std::string_view("/RunLengthArray").size();
		while (cursor < payload.size() && std::isspace(static_cast<unsigned char>(to_u8(payload[cursor]))))
		{
			++cursor;
		}
		if (cursor >= payload.size() || to_u8(payload[cursor]) != static_cast<uint8_t>('['))
		{
			search_pos = marker_pos_opt.value() + 1u;
			continue;
		}

		RunLengthArraySpan span{};
		span.open_bracket = cursor;
		++cursor;

		while (cursor < payload.size())
		{
			while (cursor < payload.size() && std::isspace(static_cast<unsigned char>(to_u8(payload[cursor]))))
			{
				++cursor;
			}

			if (cursor >= payload.size())
			{
				break;
			}

			const auto c = static_cast<char>(to_u8(payload[cursor]));
			if (c == ']')
			{
				span.close_bracket = cursor;
				arrays.push_back(std::move(span));
				cursor += 1u;
				break;
			}

			const size_t number_start = cursor;
			if (c == '-')
			{
				++cursor;
			}

			const size_t digits_start = cursor;
			while (cursor < payload.size() && std::isdigit(static_cast<unsigned char>(to_u8(payload[cursor]))))
			{
				++cursor;
			}

			if (digits_start == cursor)
			{
				cursor = number_start + 1u;
				continue;
			}

			int64_t parsed = 0;
			for (size_t i = digits_start; i < cursor; ++i)
			{
				parsed = parsed * 10 + static_cast<int64_t>(to_u8(payload[i]) - static_cast<uint8_t>('0'));
			}
			if (to_u8(payload[number_start]) == static_cast<uint8_t>('-'))
			{
				parsed = -parsed;
			}

			if (parsed >= static_cast<int64_t>(std::numeric_limits<int32_t>::min()) &&
				parsed <= static_cast<int64_t>(std::numeric_limits<int32_t>::max()))
			{
				span.values.push_back(static_cast<int32_t>(parsed));
			}
		}

		search_pos = cursor;
	}

	return arrays;
}

// -----------------------------------------------------------------------
//  Text span parsing
// -----------------------------------------------------------------------

inline std::optional<size_t> find_txt_text_value_offset(const TaggedBlock& block)
{
	static constexpr std::array<char, 8u> marker{ 'T', 'x', 't', ' ', 'T', 'E', 'X', 'T' };
	if (block.m_Data.size() < marker.size() + sizeof(uint32_t))
	{
		return std::nullopt;
	}

	for (size_t i = 0; i + marker.size() <= block.m_Data.size(); ++i)
	{
		bool match = true;
		for (size_t j = 0; j < marker.size(); ++j)
		{
			if (to_u8(block.m_Data[i + j]) != static_cast<uint8_t>(marker[j]))
			{
				match = false;
				break;
			}
		}
		if (match)
		{
			return i + marker.size();
		}
	}

	return std::nullopt;
}

inline std::optional<ParsedTextSpan> parse_text_span(const TaggedBlock& block)
{
	if (block.getKey() != Enum::TaggedBlockKey::lrTypeTool)
	{
		return std::nullopt;
	}

	const auto count_offset_opt = find_txt_text_value_offset(block);
	if (!count_offset_opt.has_value())
	{
		return std::nullopt;
	}

	const auto count_offset = count_offset_opt.value();
	if (count_offset + sizeof(uint32_t) > block.m_Data.size())
	{
		return std::nullopt;
	}

	const uint32_t code_unit_count = read_u32_be(block.m_Data, count_offset);
	const size_t value_offset = count_offset + sizeof(uint32_t);
	const size_t value_bytes = static_cast<size_t>(code_unit_count) * 2u;
	if (value_offset + value_bytes > block.m_Data.size())
	{
		return std::nullopt;
	}

	std::u16string utf16_with_null;
	utf16_with_null.reserve(code_unit_count);
	for (size_t i = 0; i < code_unit_count; ++i)
	{
		const auto high = static_cast<uint16_t>(to_u8(block.m_Data[value_offset + i * 2u]));
		const auto low = static_cast<uint16_t>(to_u8(block.m_Data[value_offset + i * 2u + 1u]));
		utf16_with_null.push_back(static_cast<char16_t>((high << 8) | low));
	}

	size_t text_units = utf16_with_null.size();
	while (text_units > 0 && utf16_with_null[text_units - 1u] == static_cast<char16_t>(0))
	{
		--text_units;
	}

	ParsedTextSpan span{};
	span.count_offset = count_offset;
	span.value_offset = value_offset;
	span.code_unit_count = code_unit_count;
	span.text_utf16_length = text_units;
	span.trailing_null_units = code_unit_count >= text_units ? code_unit_count - text_units : 0u;
	span.text_utf16 = std::u16string(utf16_with_null.begin(), utf16_with_null.begin() + static_cast<std::ptrdiff_t>(text_units));
	span.text_utf8 = UnicodeString::convertUTF16LEtoUTF8(span.text_utf16);
	return span;
}

inline bool write_text_in_span(TaggedBlock& block, const ParsedTextSpan& span, const std::u16string& utf16le)
{
	const size_t old_value_bytes = span.code_unit_count * 2u;
	const size_t new_code_unit_count = utf16le.size() + span.trailing_null_units;
	const size_t new_value_bytes = new_code_unit_count * 2u;
	const size_t old_end = span.value_offset + old_value_bytes;

	if (new_value_bytes > old_value_bytes)
	{
		block.m_Data.insert(
			block.m_Data.begin() + static_cast<std::ptrdiff_t>(old_end),
			new_value_bytes - old_value_bytes,
			static_cast<std::byte>(0)
		);
	}
	else if (new_value_bytes < old_value_bytes)
	{
		block.m_Data.erase(
			block.m_Data.begin() + static_cast<std::ptrdiff_t>(span.value_offset + new_value_bytes),
			block.m_Data.begin() + static_cast<std::ptrdiff_t>(old_end)
		);
	}

	if (new_code_unit_count > static_cast<size_t>(std::numeric_limits<uint32_t>::max()))
	{
		PSAPI_LOG_ERROR("TextLayer", "TextLayer string exceeded uint32_t code-unit limit");
		return false;
	}
	write_u32_be(block.m_Data, span.count_offset, static_cast<uint32_t>(new_code_unit_count));
	write_utf16le_as_be(block.m_Data, span.value_offset, utf16le);

	const size_t null_start = span.value_offset + utf16le.size() * 2u;
	for (size_t i = 0; i < span.trailing_null_units * 2u; ++i)
	{
		block.m_Data[null_start + i] = static_cast<std::byte>(0);
	}

	refresh_type_tool_descriptor_cache(block);
	return true;
}

// -----------------------------------------------------------------------
//  Text descriptor helpers (read/write enum values in the text descriptor)
// -----------------------------------------------------------------------

/// Return the offset of the text descriptor body inside the TySh TaggedBlock.
/// The text descriptor starts right after the 56-byte TySh header.
inline size_t find_text_descriptor_body_offset(const TaggedBlock& block)
{
	if (block.getKey() != Enum::TaggedBlockKey::lrTypeTool)
		return 0u;

	// TySh layout: 2 (version) + 6*8 (transform) + 2 (text desc version) = 52 + 4 = 56
	// But the descriptor body actually starts at offset 56 (after the 4-byte version prefix
	// of the descriptor, which is already included in the body parse).
	// skip_descriptor_body expects to start at the className (right after the 4-byte format version).
	static constexpr size_t tysh_header_bytes = 56u;
	if (block.m_Data.size() < tysh_header_bytes + 12u)
		return 0u;

	return tysh_header_bytes;
}

/// Read an enum-type key from the text descriptor.
/// Returns the enum value string (e.g. "AnCr" for anti-alias crisp).
inline std::optional<std::string> read_text_desc_enum(const TaggedBlock& block, const std::string& target_key)
{
	if (const auto* typed = dynamic_cast<const TypeToolTaggedBlock*>(&block))
	{
		if (typed->has_parsed_descriptors())
		{
			try
			{
				const auto* enumerated = typed->text_descriptor().at<Descriptors::Enumerated>(target_key);
				if (enumerated == nullptr)
				{
					return std::nullopt;
				}
				return enumerated->m_Enum;
			}
			catch (...)
			{
				return std::nullopt;
			}
		}
	}

	const size_t body = find_text_descriptor_body_offset(block);
	if (body == 0u) return std::nullopt;

	Descriptors::Descriptor descriptor{};
	if (!parse_descriptor_body(block.m_Data, body, descriptor))
	{
		return std::nullopt;
	}

	try
	{
		const auto* enumerated = descriptor.at<Descriptors::Enumerated>(target_key);
		if (enumerated == nullptr)
		{
			return std::nullopt;
		}
		return enumerated->m_Enum;
	}
	catch (...)
	{
		return std::nullopt;
	}
}

/// Write an enum-type value in the text descriptor in-place.
/// Handles variable-length value replacement by resizing the buffer when
/// the new value has a different encoded size than the old one.
/// Uses Photoshop's descriptor key encoding: length == 0 means 4-byte charID,
/// length > 0 means variable-length stringID.
/// Returns true on success.
inline bool write_text_desc_enum(TaggedBlock& block, const std::string& target_key, const std::string& new_value)
{
	if (auto* typed = dynamic_cast<TypeToolTaggedBlock*>(&block))
	{
		if (typed->has_parsed_descriptors())
		{
			try
			{
				auto& entry = typed->text_descriptor().at(target_key);
				auto* enumerated = dynamic_cast<Descriptors::Enumerated*>(entry.get());
				if (enumerated == nullptr)
				{
					return false;
				}
				enumerated->m_Enum = new_value;
				typed->sync_data_from_descriptors();
				return true;
			}
			catch (...)
			{
				return false;
			}
		}
	}

	const size_t body = find_text_descriptor_body_offset(block);
	if (body == 0u) return false;

	const size_t old_body_size = skip_descriptor_body(block.m_Data, body);
	if (old_body_size == 0u)
	{
		return false;
	}

	Descriptors::Descriptor descriptor{};
	if (!parse_descriptor_body(block.m_Data, body, descriptor))
	{
		return false;
	}

	try
	{
		auto& entry = descriptor.at(target_key);
		auto* enumerated = dynamic_cast<Descriptors::Enumerated*>(entry.get());
		if (enumerated == nullptr)
		{
			return false;
		}
		enumerated->m_Enum = new_value;
	}
	catch (...)
	{
		return false;
	}

	const auto new_body = serialize_descriptor_body(descriptor);
	block.m_Data.erase(
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(body),
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(body + old_body_size));
	block.m_Data.insert(
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(body),
		new_body.begin(),
		new_body.end());
	refresh_type_tool_descriptor_cache(block);
	return true;
}

// -----------------------------------------------------------------------
//  Warp descriptor helpers
// -----------------------------------------------------------------------

inline size_t find_warp_descriptor_body_offset(const TaggedBlock& block)
{
	if (block.getKey() != Enum::TaggedBlockKey::lrTypeTool)
		return 0u;

	static constexpr size_t tysh_header_bytes = 56u;
	if (block.m_Data.size() < tysh_header_bytes + 12u)
		return 0u;

	const size_t text_desc_size = skip_descriptor_body(block.m_Data, tysh_header_bytes);
	if (text_desc_size == 0u)
		return 0u;

	const size_t warp_header_offset = tysh_header_bytes + text_desc_size;
	static constexpr size_t warp_prefix_bytes = 6u;
	if (warp_header_offset + warp_prefix_bytes > block.m_Data.size())
		return 0u;

	return warp_header_offset + warp_prefix_bytes;
}

inline std::optional<std::string> read_warp_enum(const TaggedBlock& block, const std::string& target_key)
{
	if (const auto* typed = dynamic_cast<const TypeToolTaggedBlock*>(&block))
	{
		if (typed->has_parsed_descriptors())
		{
			try
			{
				const auto* enumerated = typed->warp_descriptor().at<Descriptors::Enumerated>(target_key);
				if (enumerated == nullptr)
				{
					return std::nullopt;
				}
				return enumerated->m_Enum;
			}
			catch (...)
			{
				return std::nullopt;
			}
		}
	}

	const size_t warp_body = find_warp_descriptor_body_offset(block);
	if (warp_body == 0u) return std::nullopt;

	Descriptors::Descriptor descriptor{};
	if (!parse_descriptor_body(block.m_Data, warp_body, descriptor))
	{
		return std::nullopt;
	}

	try
	{
		const auto* enumerated = descriptor.at<Descriptors::Enumerated>(target_key);
		if (enumerated == nullptr)
		{
			return std::nullopt;
		}
		return enumerated->m_Enum;
	}
	catch (...)
	{
		return std::nullopt;
	}
}

inline std::optional<double> read_warp_double(const TaggedBlock& block, const std::string& target_key)
{
	if (const auto* typed = dynamic_cast<const TypeToolTaggedBlock*>(&block))
	{
		if (typed->has_parsed_descriptors())
		{
			try
			{
				return typed->warp_descriptor().at<double>(target_key);
			}
			catch (...)
			{
				// fallthrough
			}
			try
			{
				const auto* unit_float = typed->warp_descriptor().at<Descriptors::UnitFloat>(target_key);
				if (unit_float == nullptr)
				{
					return std::nullopt;
				}
				return unit_float->m_Value;
			}
			catch (...)
			{
				return std::nullopt;
			}
		}
	}

	const size_t warp_body = find_warp_descriptor_body_offset(block);
	if (warp_body == 0u) return std::nullopt;

	Descriptors::Descriptor descriptor{};
	if (!parse_descriptor_body(block.m_Data, warp_body, descriptor))
	{
		return std::nullopt;
	}

	try
	{
		return descriptor.at<double>(target_key);
	}
	catch (...)
	{
		// fall through
	}

	try
	{
		const auto* unit_float = descriptor.at<Descriptors::UnitFloat>(target_key);
		if (unit_float == nullptr)
		{
			return std::nullopt;
		}
		return unit_float->m_Value;
	}
	catch (...)
	{
		return std::nullopt;
	}
}

inline bool write_warp_enum(TaggedBlock& block, const std::string& target_key, const std::string& new_value)
{
	if (auto* typed = dynamic_cast<TypeToolTaggedBlock*>(&block))
	{
		if (typed->has_parsed_descriptors())
		{
			try
			{
				auto& entry = typed->warp_descriptor().at(target_key);
				auto* enumerated = dynamic_cast<Descriptors::Enumerated*>(entry.get());
				if (enumerated == nullptr)
				{
					return false;
				}
				enumerated->m_Enum = new_value;
				typed->sync_data_from_descriptors();
				return true;
			}
			catch (...)
			{
				return false;
			}
		}
	}

	const size_t warp_body = find_warp_descriptor_body_offset(block);
	if (warp_body == 0u) return false;

	const size_t old_warp_body_size = skip_descriptor_body(block.m_Data, warp_body);
	if (old_warp_body_size == 0u)
	{
		return false;
	}

	Descriptors::Descriptor descriptor{};
	if (!parse_descriptor_body(block.m_Data, warp_body, descriptor))
	{
		return false;
	}

	try
	{
		auto& entry = descriptor.at(target_key);
		auto* enumerated = dynamic_cast<Descriptors::Enumerated*>(entry.get());
		if (enumerated == nullptr)
		{
			return false;
		}
		enumerated->m_Enum = new_value;
	}
	catch (...)
	{
		return false;
	}

	const auto new_body = serialize_descriptor_body(descriptor);
	block.m_Data.erase(
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(warp_body),
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(warp_body + old_warp_body_size));
	block.m_Data.insert(
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(warp_body),
		new_body.begin(),
		new_body.end());
	refresh_type_tool_descriptor_cache(block);
	return true;
}

inline bool write_warp_double(TaggedBlock& block, const std::string& target_key, const double new_value)
{
	if (auto* typed = dynamic_cast<TypeToolTaggedBlock*>(&block))
	{
		if (typed->has_parsed_descriptors())
		{
			try
			{
				auto& entry = typed->warp_descriptor().at(target_key);
				if (auto* double_value = dynamic_cast<Descriptors::double_Wrapper*>(entry.get()))
				{
					double_value->m_Value = new_value;
				}
				else if (auto* unit_float = dynamic_cast<Descriptors::UnitFloat*>(entry.get()))
				{
					unit_float->m_Value = new_value;
				}
				else
				{
					return false;
				}
				typed->sync_data_from_descriptors();
				return true;
			}
			catch (...)
			{
				return false;
			}
		}
	}

	const size_t warp_body = find_warp_descriptor_body_offset(block);
	if (warp_body == 0u) return false;

	const size_t old_warp_body_size = skip_descriptor_body(block.m_Data, warp_body);
	if (old_warp_body_size == 0u)
	{
		return false;
	}

	Descriptors::Descriptor descriptor{};
	if (!parse_descriptor_body(block.m_Data, warp_body, descriptor))
	{
		return false;
	}

	try
	{
		auto& entry = descriptor.at(target_key);

		if (auto* double_value = dynamic_cast<Descriptors::double_Wrapper*>(entry.get()))
		{
			double_value->m_Value = new_value;
		}
		else if (auto* unit_float = dynamic_cast<Descriptors::UnitFloat*>(entry.get()))
		{
			unit_float->m_Value = new_value;
		}
		else
		{
			return false;
		}
	}
	catch (...)
	{
		return false;
	}

	const auto new_body = serialize_descriptor_body(descriptor);
	block.m_Data.erase(
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(warp_body),
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(warp_body + old_warp_body_size));
	block.m_Data.insert(
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(warp_body),
		new_body.begin(),
		new_body.end());
	refresh_type_tool_descriptor_cache(block);
	return true;
}

} // namespace TextLayerDetail

PSAPI_NAMESPACE_END
