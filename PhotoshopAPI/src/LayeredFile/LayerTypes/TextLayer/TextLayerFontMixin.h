#pragma once

#include "Macros.h"
#include "TextLayerFwd.h"
#include "TextLayerEnum.h"
#include "TextLayerEngineDataUtils.h"
#include "TextLayerU16Utils.h"

#include "Core/Struct/EngineDataStructure.h"
#include "Core/Struct/UnicodeString.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

PSAPI_NAMESPACE_BEGIN

template <typename Derived>
struct TextLayerFontMixin
{
protected:
	Derived* self() { return static_cast<Derived*>(this); }
	const Derived* self() const { return static_cast<const Derived*>(this); }

public:

	// ================================================================
	// FontSet read APIs
	// ================================================================

	size_t font_count() const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto font_set = EngineData::find_by_path(parsed.root, { "ResourceDict", "FontSet" });
			if (font_set != nullptr && font_set->type == EngineData::ValueType::Array)
				return font_set->array_items.size();
		}
		return 0u;
	}

	std::optional<std::string> font_postscript_name(const size_t font_index) const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			return TextLayerDetail::font_property_string(parsed.root, font_index, "Name");
		}
		return std::nullopt;
	}

	std::optional<std::string> font_name(const size_t font_index) const
	{
		return font_postscript_name(font_index);
	}

	std::optional<TextLayerEnum::FontScript> font_script(const size_t font_index) const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			auto v = TextLayerDetail::font_property_int32(parsed.root, font_index, "Script");
			if (v.has_value()) return static_cast<TextLayerEnum::FontScript>(*v);
		}
		return std::nullopt;
	}

	std::optional<TextLayerEnum::FontType> font_type(const size_t font_index) const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			auto v = TextLayerDetail::font_property_int32(parsed.root, font_index, "FontType");
			if (v.has_value()) return static_cast<TextLayerEnum::FontType>(*v);
		}
		return std::nullopt;
	}

	std::optional<int32_t> font_synthetic(const size_t font_index) const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			return TextLayerDetail::font_property_int32(parsed.root, font_index, "Synthetic");
		}
		return std::nullopt;
	}

	bool is_sentinel_font(const size_t font_index) const
	{
		const auto name = font_postscript_name(font_index);
		return name.has_value() && name.value() == "AdobeInvisFont";
	}

	std::vector<size_t> used_font_indices() const
	{
		std::vector<size_t> result;
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto run_array = EngineData::find_by_path(parsed.root, { "EngineDict", "StyleRun", "RunArray" });
			if (run_array == nullptr || run_array->type != EngineData::ValueType::Array) return result;
			for (const auto& run : run_array->array_items)
			{
				const auto ssd = EngineData::find_by_path(run, { "StyleSheet", "StyleSheetData" });
				if (ssd == nullptr) continue;
				const auto font_val = EngineData::find_dict_value(*ssd, "Font");
				if (font_val == nullptr || font_val->type != EngineData::ValueType::Number) continue;
				const auto idx = static_cast<size_t>(
					font_val->is_integer ? font_val->integer_value : static_cast<int64_t>(font_val->number_value));
				if (std::find(result.begin(), result.end(), idx) == result.end())
					result.push_back(idx);
			}
			std::sort(result.begin(), result.end());
			return result;
		}
		return result;
	}

	std::vector<std::string> used_font_names() const
	{
		std::vector<std::string> result;
		const auto indices = used_font_indices();
		for (const auto idx : indices)
		{
			if (is_sentinel_font(idx)) continue;
			const auto name = font_postscript_name(idx);
			if (name.has_value() && !name.value().empty())
				result.push_back(name.value());
		}
		return result;
	}

	// -----------------------------------------------------------------------
	//  FontSet write / sync
	// -----------------------------------------------------------------------

	int32_t find_font_index(const std::string& postscript_name) const
	{
		const auto count = font_count();
		for (size_t i = 0; i < count; ++i)
		{
			const auto name = font_postscript_name(i);
			if (name.has_value() && name.value() == postscript_name)
				return static_cast<int32_t>(i);
		}
		return -1;
	}

	int32_t add_font(const std::string& postscript_name,
		const TextLayerEnum::FontType font_type_val = TextLayerEnum::FontType::OpenType,
		const TextLayerEnum::FontScript script = TextLayerEnum::FontScript::Roman,
		const int32_t synthetic = 0)
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto engine_span_opt = TextLayerDetail::find_engine_data_span(*block);
			if (!engine_span_opt.has_value()) continue;

			std::vector<std::byte> payload(
				block->m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset),
				block->m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset + engine_span_opt->byte_count)
			);

			auto parsed = EngineData::parse(payload);
			if (!parsed.ok) continue;

			auto entry = EngineData::make_dict();
			EngineData::insert_dict_value(entry, "Name",
				EngineData::make_string(TextLayerDetail::encode_engine_literal_utf16be(postscript_name)));
			EngineData::insert_dict_value(entry, "Script", EngineData::make_integer(static_cast<int32_t>(script)));
			EngineData::insert_dict_value(entry, "FontType", EngineData::make_integer(static_cast<int32_t>(font_type_val)));
			EngineData::insert_dict_value(entry, "Synthetic", EngineData::make_integer(synthetic));

			int32_t new_index = -1;
			std::vector<EngineData::PayloadPatch> patches;

			auto rd_fs = EngineData::find_by_path(parsed.root, { "ResourceDict", "FontSet" });
			if (rd_fs != nullptr && rd_fs->type == EngineData::ValueType::Array)
			{
				new_index = static_cast<int32_t>(rd_fs->array_items.size());
			}
			else
			{
				return -1;
			}

			// Insert into ResourceDict/FontSet
			if (!EngineData::insert_array_item_bytes(payload, *rd_fs, entry))
				return -1;

			// Re-parse after the first insertion so offsets are fresh for DocumentResources
			parsed = EngineData::parse(payload);
			if (!parsed.ok) return -1;

			auto doc_fs = EngineData::find_by_path(parsed.root, { "DocumentResources", "FontSet" });
			if (doc_fs != nullptr && doc_fs->type == EngineData::ValueType::Array)
				EngineData::insert_array_item_bytes(payload, *doc_fs, entry);

			if (!TextLayerDetail::write_engine_payload(*block, engine_span_opt.value(), payload))
				return -1;
			return new_index;
		}
		return -1;
	}

	bool set_font_postscript_name(const size_t font_index, const std::string& new_name)
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto engine_span_opt = TextLayerDetail::find_engine_data_span(*block);
			if (!engine_span_opt.has_value()) continue;

			std::vector<std::byte> payload(
				block->m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset),
				block->m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset + engine_span_opt->byte_count)
			);

			auto parsed = EngineData::parse(payload);
			if (!parsed.ok) continue;

			const auto encoded = TextLayerDetail::encode_engine_literal_utf16be(new_name);
			std::vector<EngineData::PayloadPatch> patches;

			auto rd_fs = EngineData::find_by_path(parsed.root, { "ResourceDict", "FontSet" });
			if (rd_fs != nullptr && rd_fs->type == EngineData::ValueType::Array
				&& font_index < rd_fs->array_items.size())
			{
				auto name_val = EngineData::find_dict_value(rd_fs->array_items[font_index], "Name");
				if (name_val != nullptr)
				{
					const size_t old_start = name_val->start_offset;
					const size_t old_end = name_val->end_offset;
					EngineData::set_string(*name_val, encoded);
					patches.push_back({ old_start, old_end, EngineData::format_value_bytes(*name_val) });
				}
			}
			if (patches.empty()) return false;

			auto doc_fs = EngineData::find_by_path(parsed.root, { "DocumentResources", "FontSet" });
			if (doc_fs != nullptr && doc_fs->type == EngineData::ValueType::Array
				&& font_index < doc_fs->array_items.size())
			{
				auto name_val = EngineData::find_dict_value(doc_fs->array_items[font_index], "Name");
				if (name_val != nullptr)
				{
					const size_t old_start = name_val->start_offset;
					const size_t old_end = name_val->end_offset;
					EngineData::set_string(*name_val, encoded);
					patches.push_back({ old_start, old_end, EngineData::format_value_bytes(*name_val) });
				}
			}

			EngineData::apply_patches(payload, patches);
			return TextLayerDetail::write_engine_payload(*block, engine_span_opt.value(), payload);
		}
		return false;
	}

	bool set_style_run_font_by_name(const size_t run_index, const std::string& postscript_name)
	{
		int32_t idx = find_font_index(postscript_name);
		if (idx < 0) idx = add_font(postscript_name);
		if (idx < 0) return false;
		return self()->set_style_run_font(run_index, idx);
	}

	bool set_style_normal_font_by_name(const std::string& postscript_name)
	{
		int32_t idx = find_font_index(postscript_name);
		if (idx < 0) idx = add_font(postscript_name);
		if (idx < 0) return false;
		return self()->set_style_normal_font(idx);
	}

	// -----------------------------------------------------------------------
	//  High-level font convenience
	// -----------------------------------------------------------------------

	std::optional<std::string> primary_font_name() const
	{
		const auto idx = self()->style_run_font(0);
		if (idx.has_value())
			return font_postscript_name(static_cast<size_t>(idx.value()));
		const auto nidx = self()->style_normal_font();
		if (nidx.has_value())
			return font_postscript_name(static_cast<size_t>(nidx.value()));
		return std::nullopt;
	}

	bool set_font(const std::string& postscript_name)
	{
		int32_t idx = find_font_index(postscript_name);
		if (idx < 0) idx = add_font(postscript_name);
		if (idx < 0) return false;

		bool ok = true;
		const auto count = self()->style_run_count();
		for (size_t i = 0; i < count; ++i)
		{
			if (!self()->set_style_run_font(i, idx))
				ok = false;
		}
		if (!self()->set_style_normal_font(idx))
			ok = false;
		return ok;
	}
};

PSAPI_NAMESPACE_END
