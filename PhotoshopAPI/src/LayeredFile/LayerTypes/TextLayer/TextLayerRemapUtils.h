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
	const size_t old_body_size = skip_descriptor_body(block.m_Data, tysh_header_bytes);
	if (old_body_size == 0u)
	{
		return false;
	}

	Descriptors::Descriptor descriptor{};
	if (!parse_descriptor_body(block.m_Data, tysh_header_bytes, descriptor))
	{
		return false;
	}

	std::function<void(const std::string&, Descriptors::DescriptorBase*)> remap_value;
	remap_value = [&](const std::string& item_key, Descriptors::DescriptorBase* value)
	{
		if (value == nullptr)
		{
			return;
		}

		if (item_key == "From" || item_key == "T   ")
		{
			auto* int32_value = dynamic_cast<Descriptors::int32_t_Wrapper*>(value);
			if (int32_value != nullptr && int32_value->m_Value >= 0)
			{
				const auto remapped = remap_engine_index(
					static_cast<size_t>(int32_value->m_Value),
					old_plain_units,
					replacements
				);
				if (remapped <= static_cast<size_t>(std::numeric_limits<int32_t>::max()))
				{
					int32_value->m_Value = static_cast<int32_t>(remapped);
				}
			}
		}

		if (auto* nested_descriptor = dynamic_cast<Descriptors::Descriptor*>(value))
		{
			for (auto& [nested_key, nested_value] : nested_descriptor->items())
			{
				remap_value(nested_key, nested_value.get());
			}
			return;
		}

		if (auto* object_array = dynamic_cast<Descriptors::ObjectArray*>(value))
		{
			for (auto& [nested_key, nested_value] : object_array->items())
			{
				remap_value(nested_key, nested_value.get());
			}
			return;
		}

		if (auto* list = dynamic_cast<Descriptors::List*>(value))
		{
			for (auto& item : list->m_Items)
			{
				remap_value("", item.get());
			}
		}
	};

	for (auto& [key, value] : descriptor.items())
	{
		remap_value(key, value.get());
	}

	const auto new_body = serialize_descriptor_body(descriptor);
	block.m_Data.erase(
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(tysh_header_bytes),
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(tysh_header_bytes + old_body_size));
	block.m_Data.insert(
		block.m_Data.begin() + static_cast<std::ptrdiff_t>(tysh_header_bytes),
		new_body.begin(),
		new_body.end());
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
