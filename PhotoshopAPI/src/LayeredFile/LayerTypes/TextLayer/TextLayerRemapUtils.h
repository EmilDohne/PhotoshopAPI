#pragma once

// =========================================================================
//  TextLayerRemapUtils.h
//
//  Text mutation / remapping utilities for TextLayer.
//  Handles updating EngineData RunLengthArrays and legacy descriptor
//  From/To ranges when text content changes.
// =========================================================================

#include "Macros.h"
#include "TextLayerTypes.h"
#include "TextLayerParsingUtils.h"
#include "TextLayerU16Utils.h"
#include "TextLayerDescriptorUtils.h"
#include "TextLayerEngineDataUtils.h"

#include "Core/Struct/EngineDataStructure.h"
#include "PhotoshopFile/AdditionalLayerInfo.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include <vector>

PSAPI_NAMESPACE_BEGIN

namespace TextLayerDetail
{

// -----------------------------------------------------------------------
//  Remap helpers (EngineData + legacy descriptor)
// -----------------------------------------------------------------------

inline bool remap_engine_data(
	TaggedBlock& block,
	const ParsedTextSpan& old_span,
	const std::u16string& new_text_utf16,
	const std::vector<TextReplacement>& replacements)
{
	const auto engine_span_opt = find_engine_data_span(block);
	if (!engine_span_opt.has_value())
	{
		return false;
	}
	const auto engine_span = engine_span_opt.value();

	std::vector<std::byte> payload(
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span.value_offset),
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span.value_offset + engine_span.byte_count)
	);

	auto parsed = EngineData::parse(payload);
	if (!parsed.ok)
	{
		return false;
	}

	auto editor_text = EngineData::find_by_path(parsed.root, { "EngineDict", "Editor", "Text" });
	if (editor_text == nullptr || editor_text->type != EngineData::ValueType::LiteralString)
	{
		return false;
	}

	std::vector<std::byte> old_engine_text_bytes{};
	old_engine_text_bytes.reserve(editor_text->string_value.size());
	for (const auto c : editor_text->string_value)
	{
		old_engine_text_bytes.push_back(static_cast<std::byte>(static_cast<uint8_t>(c)));
	}

	bool has_bom = false;
	auto old_engine_utf16 = decode_utf16be_bytes(old_engine_text_bytes, has_bom);
	if (old_engine_utf16.size() < old_span.text_utf16.size())
	{
		return false;
	}
	if (!std::equal(old_span.text_utf16.begin(), old_span.text_utf16.end(), old_engine_utf16.begin()))
	{
		return false;
	}

	const auto tail_begin = old_engine_utf16.begin() + static_cast<std::ptrdiff_t>(old_span.text_utf16.size());
	std::u16string new_engine_utf16 = new_text_utf16;
	new_engine_utf16.append(tail_begin, old_engine_utf16.end());

	auto new_engine_text_bytes = encode_utf16be_bytes(new_engine_utf16, has_bom);

	// Collect in-place patches instead of re-serializing the entire tree.
	std::vector<EngineData::PayloadPatch> patches;

	// Patch 1: the /Text literal string value
	const size_t text_old_start = editor_text->start_offset;
	const size_t text_old_end = editor_text->end_offset;
	editor_text->string_value.clear();
	editor_text->string_value.reserve(new_engine_text_bytes.size());
	for (const auto b : new_engine_text_bytes)
	{
		editor_text->string_value.push_back(static_cast<char>(to_u8(b)));
	}
	patches.push_back({ text_old_start, text_old_end, EngineData::format_value_bytes(*editor_text) });

	const auto remap_run_length_arrays = [&](auto&& self, EngineData::Value& node) -> void
	{
		if (node.type == EngineData::ValueType::Dictionary)
		{
			for (auto& [key, child] : node.dictionary_items)
			{
				if (key == "RunLengthArray" && child.type == EngineData::ValueType::Array)
				{
					std::vector<int32_t> run_lengths{};
					if (EngineData::as_int32_vector(child, run_lengths) && !run_lengths.empty())
					{
						size_t sum = 0u;
						bool valid = true;
						for (const auto value : run_lengths)
						{
							if (value < 0)
							{
								valid = false;
								break;
							}
							sum += static_cast<size_t>(value);
						}

						if (valid && sum == old_engine_utf16.size())
						{
							auto remapped = remap_run_lengths(run_lengths, old_span.text_utf16.size(), replacements);
							if (!remapped.empty())
							{
								const size_t arr_old_start = child.start_offset;
								const size_t arr_old_end = child.end_offset;
								(void)EngineData::set_int32_array(child, remapped);
								patches.push_back({ arr_old_start, arr_old_end, EngineData::format_value_bytes(child) });
							}
						}
					}
				}

				self(self, child);
			}
			return;
		}

		if (node.type == EngineData::ValueType::Array)
		{
			for (auto& item : node.array_items)
			{
				self(self, item);
			}
		}
	};

	remap_run_length_arrays(remap_run_length_arrays, parsed.root);
	EngineData::apply_patches(payload, patches);
	return write_engine_payload(block, engine_span, payload);
}

/// Remap legacy descriptor From/To integer ranges after text mutation.
/// Uses the shared descriptor walking utilities instead of duplicating them.
inline bool remap_legacy_from_to_ranges(
	TaggedBlock& block,
	const ParsedTextSpan& old_span,
	const std::vector<TextReplacement>& replacements)
{
	if (block.getKey() != Enum::TaggedBlockKey::lrTypeTool)
	{
		return false;
	}

	static constexpr size_t tysh_header_bytes = 56u;
	if (block.m_Data.size() < tysh_header_bytes + 12u)
	{
		return false;
	}

	const size_t old_plain_units = old_span.text_utf16.size();

	// Walk the descriptor tree, remapping From/To long values in-place.
	// We reuse skip_descriptor_value() from DescriptorUtils to advance the
	// cursor, and read_descriptor_key() for key parsing; only the mutation
	// logic (remap_value / remap_desc_body) is local.

	std::function<void(size_t)> remap_desc_body;
	std::function<void(size_t, const std::string&)> remap_value;

	remap_desc_body = [&](size_t pos)
	{
		if (pos + 4u > block.m_Data.size()) return;
		const uint32_t cn = read_u32_be(block.m_Data, pos); pos += 4u;
		pos += static_cast<size_t>(cn) * 2u;
		// classID
		auto [cid, cb] = read_descriptor_key(block.m_Data, pos);
		if (cb == 0u) return;
		pos += cb;
		// count
		if (pos + 4u > block.m_Data.size()) return;
		const uint32_t count = read_u32_be(block.m_Data, pos); pos += 4u;
		for (uint32_t i = 0u; i < count; ++i)
		{
			auto [k, kb] = read_descriptor_key(block.m_Data, pos);
			if (kb == 0u) return;
			pos += kb;
			remap_value(pos, k);
			const size_t skipped = skip_descriptor_value(block.m_Data, pos);
			if (skipped == 0u) return;
			pos += skipped;
		}
	};

	remap_value = [&](size_t pos, const std::string& item_key)
	{
		if (pos + 4u > block.m_Data.size()) return;
		const char t0 = static_cast<char>(to_u8(block.m_Data[pos]));
		const char t1 = static_cast<char>(to_u8(block.m_Data[pos + 1u]));
		const char t2 = static_cast<char>(to_u8(block.m_Data[pos + 2u]));
		const char t3 = static_cast<char>(to_u8(block.m_Data[pos + 3u]));

		auto match4 = [](char a, char b, char c, char d, const char* s) {
			return a == s[0] && b == s[1] && c == s[2] && d == s[3];
		};

		if (match4(t0, t1, t2, t3, "long"))
		{
			if (item_key == "From" || item_key == "T   ")
			{
				const size_t value_offset = pos + 4u;
				if (value_offset + 4u > block.m_Data.size()) return;
				const auto old_value = read_i32_be(block.m_Data, value_offset);
				if (old_value < 0) return;
				const auto remapped = remap_engine_index(
					static_cast<size_t>(old_value),
					old_plain_units,
					replacements
				);
				if (remapped <= static_cast<size_t>(std::numeric_limits<int32_t>::max()))
				{
					write_i32_be(block.m_Data, value_offset, static_cast<int32_t>(remapped));
				}
			}
		}
		else if (match4(t0, t1, t2, t3, "Objc") || match4(t0, t1, t2, t3, "GlbO") || match4(t0, t1, t2, t3, "ObAr"))
		{
			remap_desc_body(pos + 4u);
		}
		else if (match4(t0, t1, t2, t3, "VlLs"))
		{
			size_t p = pos + 4u;
			if (p + 4u > block.m_Data.size()) return;
			const uint32_t cnt = read_u32_be(block.m_Data, p); p += 4u;
			for (uint32_t i = 0u; i < cnt; ++i)
			{
				remap_value(p, item_key);
				const size_t c = skip_descriptor_value(block.m_Data, p);
				if (c == 0u) return;
				p += c;
			}
		}
	};

	remap_desc_body(tysh_header_bytes);
	return true;
}

inline bool apply_text_mutation(
	TaggedBlock& block,
	const ParsedTextSpan& parsed_old,
	const std::u16string& new_text_utf16,
	const std::vector<TextReplacement>& replacements)
{
	if (!write_text_in_span(block, parsed_old, new_text_utf16))
	{
		return false;
	}

	(void)remap_engine_data(block, parsed_old, new_text_utf16, replacements);
	(void)remap_legacy_from_to_ranges(block, parsed_old, replacements);
	return true;
}

} // namespace TextLayerDetail

PSAPI_NAMESPACE_END
