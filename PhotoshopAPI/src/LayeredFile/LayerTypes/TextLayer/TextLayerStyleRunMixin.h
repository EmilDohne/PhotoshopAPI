#pragma once

#include "Macros.h"
#include "TextLayerFwd.h"
#include "TextLayerEnum.h"
#include "TextLayerEngineDataUtils.h"

#include "Core/Struct/EngineDataStructure.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

PSAPI_NAMESPACE_BEGIN

template <typename Derived>
struct TextLayerStyleRunMixin
{
protected:
	Derived* self() { return static_cast<Derived*>(this); }
	const Derived* self() const { return static_cast<const Derived*>(this); }

public:

	size_t style_run_count() const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto run_array = EngineData::find_by_path(parsed.root, { "EngineDict", "StyleRun", "RunArray" });
			if (run_array != nullptr && run_array->type == EngineData::ValueType::Array)
				return run_array->array_items.size();
		}
		return 0u;
	}

	// --- Style run getters ---
	std::optional<double>  style_run_font_size(const size_t i) const            { return style_run_number_property(i, "FontSize"); }
	std::optional<double>  style_run_leading(const size_t i) const              { return style_run_number_property(i, "Leading"); }
	std::optional<bool>    style_run_auto_leading(const size_t i) const         { return style_run_bool_property(i, "AutoLeading"); }
	std::optional<int32_t> style_run_kerning(const size_t i) const              { return style_run_int32_property(i, "Kerning"); }
	std::optional<int32_t> style_run_font(const size_t i) const                 { return style_run_int32_property(i, "Font"); }
	std::optional<bool>    style_run_faux_bold(const size_t i) const            { return style_run_bool_property(i, "FauxBold"); }
	std::optional<bool>    style_run_faux_italic(const size_t i) const          { return style_run_bool_property(i, "FauxItalic"); }
	std::optional<double>  style_run_horizontal_scale(const size_t i) const     { return style_run_number_property(i, "HorizontalScale"); }
	std::optional<double>  style_run_vertical_scale(const size_t i) const       { return style_run_number_property(i, "VerticalScale"); }
	std::optional<int32_t> style_run_tracking(const size_t i) const             { return style_run_int32_property(i, "Tracking"); }
	std::optional<bool>    style_run_auto_kerning(const size_t i) const         { return style_run_bool_property(i, "AutoKerning"); }
	std::optional<double>  style_run_baseline_shift(const size_t i) const       { return style_run_number_property(i, "BaselineShift"); }
	std::optional<TextLayerEnum::FontCaps> style_run_font_caps(const size_t i) const              { return TextLayerEnum::opt_enum<TextLayerEnum::FontCaps>(style_run_int32_property(i, "FontCaps")); }
	std::optional<bool>    style_run_no_break(const size_t i) const              { return style_run_bool_property(i, "NoBreak"); }
	std::optional<TextLayerEnum::FontBaseline> style_run_font_baseline(const size_t i) const      { return TextLayerEnum::opt_enum<TextLayerEnum::FontBaseline>(style_run_int32_property(i, "FontBaseline")); }
	std::optional<int32_t> style_run_language(const size_t i) const             { return style_run_int32_property(i, "Language"); }
	std::optional<TextLayerEnum::CharacterDirection> style_run_character_direction(const size_t i) const { return TextLayerEnum::opt_enum<TextLayerEnum::CharacterDirection>(style_run_int32_property(i, "CharacterDirection")); }
	std::optional<TextLayerEnum::BaselineDirection> style_run_baseline_direction(const size_t i) const   { return TextLayerEnum::opt_enum<TextLayerEnum::BaselineDirection>(style_run_int32_property(i, "BaselineDirection")); }
	std::optional<double>  style_run_tsume(const size_t i) const                { return style_run_number_property(i, "Tsume"); }
	std::optional<int32_t> style_run_kashida(const size_t i) const              { return style_run_int32_property(i, "Kashida"); }
	std::optional<TextLayerEnum::DiacriticPosition> style_run_diacritic_pos(const size_t i) const        { return TextLayerEnum::opt_enum<TextLayerEnum::DiacriticPosition>(style_run_int32_property(i, "DiacriticPos")); }
	std::optional<bool>    style_run_ligatures(const size_t i) const            { return style_run_bool_property(i, "Ligatures"); }
	std::optional<bool>    style_run_dligatures(const size_t i) const           { return style_run_bool_property(i, "DLigatures"); }
	std::optional<bool>    style_run_underline(const size_t i) const            { return style_run_bool_property(i, "Underline"); }
	std::optional<bool>    style_run_strikethrough(const size_t i) const        { return style_run_bool_property(i, "Strikethrough"); }
	std::optional<bool>    style_run_stroke_flag(const size_t i) const          { return style_run_bool_property(i, "StrokeFlag"); }
	std::optional<bool>    style_run_fill_flag(const size_t i) const            { return style_run_bool_property(i, "FillFlag"); }
	std::optional<bool>    style_run_fill_first(const size_t i) const           { return style_run_bool_property(i, "FillFirst"); }
	std::optional<double>  style_run_outline_width(const size_t i) const        { return style_run_number_property(i, "OutlineWidth"); }

	std::optional<std::vector<double>> style_run_fill_color(const size_t i) const   { return style_run_color_values_property(i, "FillColor"); }
	std::optional<std::vector<double>> style_run_stroke_color(const size_t i) const { return style_run_color_values_property(i, "StrokeColor"); }

	// --- Style run setters ---
	bool set_style_run_font_size(const size_t i, const double v)
	{
		if (!std::isfinite(v) || v <= 0.0) return false;
		return set_style_run_number_property(i, "FontSize", v);
	}
	bool set_style_run_leading(const size_t i, const double v)                            { return set_style_run_number_property(i, "Leading", v); }
	bool set_style_run_auto_leading(const size_t i, const bool v)                         { return set_style_run_bool_property(i, "AutoLeading", v); }
	bool set_style_run_kerning(const size_t i, const int32_t v)                           { return set_style_run_int32_property(i, "Kerning", v); }
	bool set_style_run_font(const size_t i, const int32_t v)                              { return set_style_run_int32_property(i, "Font", v); }
	bool set_style_run_faux_bold(const size_t i, const bool v)                            { return set_style_run_bool_property(i, "FauxBold", v); }
	bool set_style_run_faux_italic(const size_t i, const bool v)                          { return set_style_run_bool_property(i, "FauxItalic", v); }
	bool set_style_run_horizontal_scale(const size_t i, const double v)                   { return set_style_run_number_property(i, "HorizontalScale", v); }
	bool set_style_run_vertical_scale(const size_t i, const double v)                     { return set_style_run_number_property(i, "VerticalScale", v); }
	bool set_style_run_tracking(const size_t i, const int32_t v)                          { return set_style_run_int32_property(i, "Tracking", v); }
	bool set_style_run_auto_kerning(const size_t i, const bool v)                         { return set_style_run_bool_property(i, "AutoKerning", v); }
	bool set_style_run_baseline_shift(const size_t i, const double v)                     { return set_style_run_number_property(i, "BaselineShift", v); }
	bool set_style_run_font_caps(const size_t i, const TextLayerEnum::FontCaps v)              { return set_style_run_int32_property(i, "FontCaps", static_cast<int32_t>(v)); }
	bool set_style_run_no_break(const size_t i, const bool v)                             { return set_style_run_bool_property(i, "NoBreak", v); }
	bool set_style_run_font_baseline(const size_t i, const TextLayerEnum::FontBaseline v)      { return set_style_run_int32_property(i, "FontBaseline", static_cast<int32_t>(v)); }
	bool set_style_run_language(const size_t i, const int32_t v)                          { return set_style_run_int32_property(i, "Language", v); }
	bool set_style_run_character_direction(const size_t i, const TextLayerEnum::CharacterDirection v) { return set_style_run_int32_property(i, "CharacterDirection", static_cast<int32_t>(v)); }
	bool set_style_run_baseline_direction(const size_t i, const TextLayerEnum::BaselineDirection v)   { return set_style_run_int32_property(i, "BaselineDirection", static_cast<int32_t>(v)); }
	bool set_style_run_tsume(const size_t i, const double v)                              { return set_style_run_number_property(i, "Tsume", v); }
	bool set_style_run_kashida(const size_t i, const int32_t v)                           { return set_style_run_int32_property(i, "Kashida", v); }
	bool set_style_run_diacritic_pos(const size_t i, const TextLayerEnum::DiacriticPosition v) { return set_style_run_int32_property(i, "DiacriticPos", static_cast<int32_t>(v)); }
	bool set_style_run_ligatures(const size_t i, const bool v)                            { return set_style_run_bool_property(i, "Ligatures", v); }
	bool set_style_run_dligatures(const size_t i, const bool v)                           { return set_style_run_bool_property(i, "DLigatures", v); }
	bool set_style_run_underline(const size_t i, const bool v)                            { return set_style_run_bool_property(i, "Underline", v); }
	bool set_style_run_strikethrough(const size_t i, const bool v)                        { return set_style_run_bool_property(i, "Strikethrough", v); }
	bool set_style_run_stroke_flag(const size_t i, const bool v)                          { return set_style_run_bool_property(i, "StrokeFlag", v); }
	bool set_style_run_fill_flag(const size_t i, const bool v)                            { return set_style_run_bool_property(i, "FillFlag", v); }
	bool set_style_run_fill_first(const size_t i, const bool v)                           { return set_style_run_bool_property(i, "FillFirst", v); }
	bool set_style_run_outline_width(const size_t i, const double v)                      { return set_style_run_number_property(i, "OutlineWidth", v); }

	bool set_style_run_fill_color(const size_t i, const std::vector<double>& v)           { return set_style_run_color_values_property(i, "FillColor", v); }
	bool set_style_run_stroke_color(const size_t i, const std::vector<double>& v)         { return set_style_run_color_values_property(i, "StrokeColor", v); }

private:

	// -----------------------------------------------------------------------
	//  Property helpers
	// -----------------------------------------------------------------------

	std::optional<double> style_run_number_property(const size_t run_index, const std::string_view property_key) const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto style_data = TextLayerDetail::style_run_style_data_for_run(parsed.root, run_index);
			if (style_data == nullptr) continue;
			const auto value = EngineData::find_dict_value(*style_data, property_key);
			if (value == nullptr || value->type != EngineData::ValueType::Number) return std::nullopt;
			return value->number_value;
		}
		return std::nullopt;
	}

	std::optional<int32_t> style_run_int32_property(const size_t run_index, const std::string_view property_key) const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto style_data = TextLayerDetail::style_run_style_data_for_run(parsed.root, run_index);
			if (style_data == nullptr) continue;
			const auto value = EngineData::find_dict_value(*style_data, property_key);
			int32_t parsed_value = 0;
			if (value == nullptr || !TextLayerDetail::number_value_to_int32(*value, parsed_value)) return std::nullopt;
			return parsed_value;
		}
		return std::nullopt;
	}

	std::optional<bool> style_run_bool_property(const size_t run_index, const std::string_view property_key) const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto style_data = TextLayerDetail::style_run_style_data_for_run(parsed.root, run_index);
			if (style_data == nullptr) continue;
			const auto value = EngineData::find_dict_value(*style_data, property_key);
			if (value == nullptr || value->type != EngineData::ValueType::Boolean) return std::nullopt;
			return value->bool_value;
		}
		return std::nullopt;
	}

	std::optional<std::vector<double>> style_run_color_values_property(const size_t run_index, const std::string_view color_key) const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto style_data = TextLayerDetail::style_run_style_data_for_run(parsed.root, run_index);
			if (style_data == nullptr) continue;
			const auto color_dict = EngineData::find_dict_value(*style_data, color_key);
			if (color_dict == nullptr) return std::nullopt;
			const auto values = EngineData::find_dict_value(*color_dict, "Values");
			if (values == nullptr) return std::nullopt;
			std::vector<double> out_values{};
			if (!EngineData::as_double_vector(*values, out_values) || out_values.empty()) return std::nullopt;
			return out_values;
		}
		return std::nullopt;
	}

	bool set_style_run_number_property(const size_t run_index, const std::string_view property_key, const double value)
	{
		if (!std::isfinite(value)) return false;
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto engine_span_opt = TextLayerDetail::find_engine_data_span(*block);
			if (!engine_span_opt.has_value()) continue;
			std::vector<std::byte> payload(
				block->m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset),
				block->m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset + engine_span_opt->byte_count));
			auto parsed = EngineData::parse(payload);
			if (!parsed.ok) continue;
			auto style_data = TextLayerDetail::style_run_style_data_for_run(parsed.root, run_index);
			if (style_data == nullptr) continue;
			auto property_value = EngineData::find_dict_value(*style_data, property_key);
			if (property_value == nullptr)
			{
				if (!EngineData::insert_dict_entry_bytes(payload, *style_data, std::string(property_key), EngineData::make_number(value)))
					return false;
			}
			else
			{
				const size_t old_start = property_value->start_offset;
				const size_t old_end = property_value->end_offset;
				if (!EngineData::set_number(*property_value, value)) return false;
				auto new_bytes = EngineData::format_value_bytes(*property_value);
				EngineData::splice_payload(payload, old_start, old_end, new_bytes);
			}
			return TextLayerDetail::write_engine_payload(*block, engine_span_opt.value(), payload);
		}
		return false;
	}

	bool set_style_run_int32_property(const size_t run_index, const std::string_view property_key, const int32_t value)
	{
		return set_style_run_number_property(run_index, property_key, static_cast<double>(value));
	}

	bool set_style_run_bool_property(const size_t run_index, const std::string_view property_key, const bool value)
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto engine_span_opt = TextLayerDetail::find_engine_data_span(*block);
			if (!engine_span_opt.has_value()) continue;
			std::vector<std::byte> payload(
				block->m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset),
				block->m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset + engine_span_opt->byte_count));
			auto parsed = EngineData::parse(payload);
			if (!parsed.ok) continue;
			auto style_data = TextLayerDetail::style_run_style_data_for_run(parsed.root, run_index);
			if (style_data == nullptr) continue;
			auto property_value = EngineData::find_dict_value(*style_data, property_key);
			if (property_value == nullptr)
			{
				if (!EngineData::insert_dict_entry_bytes(payload, *style_data, std::string(property_key), EngineData::make_bool(value)))
					return false;
			}
			else
			{
				const size_t old_start = property_value->start_offset;
				const size_t old_end = property_value->end_offset;
				if (property_value->type == EngineData::ValueType::Boolean)
				{
					property_value->bool_value = value;
				}
				else
				{
					*property_value = EngineData::make_bool(value);
				}
				auto new_bytes = EngineData::format_value_bytes(*property_value);
				EngineData::splice_payload(payload, old_start, old_end, new_bytes);
			}
			return TextLayerDetail::write_engine_payload(*block, engine_span_opt.value(), payload);
		}
		return false;
	}

	bool set_style_run_color_values_property(const size_t run_index, const std::string_view color_key, const std::vector<double>& values)
	{
		if (values.size() != 4u) return false;
		for (const auto v : values) { if (!std::isfinite(v)) return false; }
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto engine_span_opt = TextLayerDetail::find_engine_data_span(*block);
			if (!engine_span_opt.has_value()) continue;
			std::vector<std::byte> payload(
				block->m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset),
				block->m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset + engine_span_opt->byte_count));
			auto parsed = EngineData::parse(payload);
			if (!parsed.ok) continue;
			auto style_data = TextLayerDetail::style_run_style_data_for_run(parsed.root, run_index);
			if (style_data == nullptr) continue;
			auto color_dict = EngineData::find_dict_value(*style_data, color_key);
			if (color_dict == nullptr) return false;
			auto values_node = EngineData::find_dict_value(*color_dict, "Values");
			if (values_node == nullptr) return false;
			const size_t old_start = values_node->start_offset;
			const size_t old_end = values_node->end_offset;
			if (!EngineData::set_double_array(*values_node, values)) return false;
			for (auto& item : values_node->array_items) item.is_integer = false;
			auto new_bytes = EngineData::format_value_bytes(*values_node);
			EngineData::splice_payload(payload, old_start, old_end, new_bytes);
			return TextLayerDetail::write_engine_payload(*block, engine_span_opt.value(), payload);
		}
		return false;
	}
};

PSAPI_NAMESPACE_END
