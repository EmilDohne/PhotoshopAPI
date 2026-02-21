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
struct TextLayerStyleNormalMixin
{
protected:
	Derived* self() { return static_cast<Derived*>(this); }
	const Derived* self() const { return static_cast<const Derived*>(this); }

public:

	/// Number of style sheets found in EngineData /ResourceDict /StyleSheetSet.
	size_t style_sheet_count() const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto sheet_set = EngineData::find_by_path(parsed.root, { "ResourceDict", "StyleSheetSet" });
			if (sheet_set != nullptr && sheet_set->type == EngineData::ValueType::Array)
				return sheet_set->array_items.size();
		}
		return 0u;
	}

	/// Retrieve /TheNormalStyleSheet index from EngineData /ResourceDict.
	std::optional<int32_t> style_normal_sheet_index() const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			int32_t normal_index = 0;
			if (!TextLayerDetail::style_normal_sheet_index_from_root(parsed.root, normal_index)) continue;
			return normal_index;
		}
		return std::nullopt;
	}

	// --- Style normal getters ---
	std::optional<int32_t>                   style_normal_font() const                 { return style_normal_int32_property("Font"); }
	std::optional<double>                    style_normal_font_size() const             { return style_normal_number_property("FontSize"); }
	std::optional<double>                    style_normal_leading() const               { return style_normal_number_property("Leading"); }
	std::optional<bool>                      style_normal_auto_leading() const          { return style_normal_bool_property("AutoLeading"); }
	std::optional<int32_t>                   style_normal_kerning() const               { return style_normal_int32_property("Kerning"); }
	std::optional<bool>                      style_normal_faux_bold() const             { return style_normal_bool_property("FauxBold"); }
	std::optional<bool>                      style_normal_faux_italic() const           { return style_normal_bool_property("FauxItalic"); }
	std::optional<double>                    style_normal_horizontal_scale() const      { return style_normal_number_property("HorizontalScale"); }
	std::optional<double>                    style_normal_vertical_scale() const        { return style_normal_number_property("VerticalScale"); }
	std::optional<int32_t>                   style_normal_tracking() const              { return style_normal_int32_property("Tracking"); }
	std::optional<bool>                      style_normal_auto_kerning() const          { return style_normal_bool_property("AutoKerning"); }
	std::optional<double>                    style_normal_baseline_shift() const        { return style_normal_number_property("BaselineShift"); }
	std::optional<TextLayerEnum::FontCaps>           style_normal_font_caps() const             { return TextLayerEnum::opt_enum<TextLayerEnum::FontCaps>(style_normal_int32_property("FontCaps")); }
	std::optional<TextLayerEnum::FontBaseline>       style_normal_font_baseline() const         { return TextLayerEnum::opt_enum<TextLayerEnum::FontBaseline>(style_normal_int32_property("FontBaseline")); }
	std::optional<bool>                      style_normal_no_break() const              { return style_normal_bool_property("NoBreak"); }
	std::optional<int32_t>                   style_normal_language() const              { return style_normal_int32_property("Language"); }
	std::optional<TextLayerEnum::CharacterDirection> style_normal_character_direction() const   { return TextLayerEnum::opt_enum<TextLayerEnum::CharacterDirection>(style_normal_int32_property("CharacterDirection")); }
	std::optional<TextLayerEnum::BaselineDirection>  style_normal_baseline_direction() const    { return TextLayerEnum::opt_enum<TextLayerEnum::BaselineDirection>(style_normal_int32_property("BaselineDirection")); }
	std::optional<double>                    style_normal_tsume() const                 { return style_normal_number_property("Tsume"); }
	std::optional<int32_t>                   style_normal_kashida() const               { return style_normal_int32_property("Kashida"); }
	std::optional<TextLayerEnum::DiacriticPosition>  style_normal_diacritic_pos() const         { return TextLayerEnum::opt_enum<TextLayerEnum::DiacriticPosition>(style_normal_int32_property("DiacriticPos")); }
	std::optional<bool>                      style_normal_ligatures() const             { return style_normal_bool_property("Ligatures"); }
	std::optional<bool>                      style_normal_dligatures() const            { return style_normal_bool_property("DLigatures"); }
	std::optional<bool>                      style_normal_underline() const             { return style_normal_bool_property("Underline"); }
	std::optional<bool>                      style_normal_strikethrough() const         { return style_normal_bool_property("Strikethrough"); }
	std::optional<bool>                      style_normal_stroke_flag() const           { return style_normal_bool_property("StrokeFlag"); }
	std::optional<bool>                      style_normal_fill_flag() const             { return style_normal_bool_property("FillFlag"); }
	std::optional<bool>                      style_normal_fill_first() const            { return style_normal_bool_property("FillFirst"); }
	std::optional<double>                    style_normal_outline_width() const         { return style_normal_number_property("OutlineWidth"); }
	std::optional<std::vector<double>>       style_normal_fill_color() const            { return style_normal_color_values_property("FillColor"); }
	std::optional<std::vector<double>>       style_normal_stroke_color() const          { return style_normal_color_values_property("StrokeColor"); }

	// --- Style normal setters ---
	bool set_style_normal_sheet_index(const int32_t sheet_index)
	{
		if (sheet_index < 0) return false;
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto engine_span_opt = TextLayerDetail::find_engine_data_span(*block);
			if (!engine_span_opt.has_value()) continue;
			std::vector<std::byte> payload(
				block->m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset),
				block->m_Data.begin() + static_cast<std::ptrdiff_t>(engine_span_opt->value_offset + engine_span_opt->byte_count));
			auto parsed = EngineData::parse(payload);
			if (!parsed.ok) continue;
			auto sheet_set = TextLayerDetail::style_sheet_set_from_root(parsed.root);
			if (sheet_set == nullptr || sheet_set->type != EngineData::ValueType::Array) continue;
			if (static_cast<size_t>(sheet_index) >= sheet_set->array_items.size()) return false;
			auto normal_index_value = EngineData::find_by_path(parsed.root, { "ResourceDict", "TheNormalStyleSheet" });
			if (normal_index_value == nullptr) return false;
			const size_t old_start = normal_index_value->start_offset;
			const size_t old_end = normal_index_value->end_offset;
			if (!EngineData::set_number(*normal_index_value, static_cast<double>(sheet_index)))
				return false;
			auto new_bytes = EngineData::format_value_bytes(*normal_index_value);
			EngineData::splice_payload(payload, old_start, old_end, new_bytes);
			return TextLayerDetail::write_engine_payload(*block, engine_span_opt.value(), payload);
		}
		return false;
	}

	bool set_style_normal_font(const int32_t v)                                    { return set_style_normal_int32_property("Font", v); }
	bool set_style_normal_font_size(const double v)                                { return set_style_normal_number_property("FontSize", v); }
	bool set_style_normal_leading(const double v)                                  { return set_style_normal_number_property("Leading", v); }
	bool set_style_normal_auto_leading(const bool v)                               { return set_style_normal_bool_property("AutoLeading", v); }
	bool set_style_normal_kerning(const int32_t v)                                 { return set_style_normal_int32_property("Kerning", v); }
	bool set_style_normal_faux_bold(const bool v)                                  { return set_style_normal_bool_property("FauxBold", v); }
	bool set_style_normal_faux_italic(const bool v)                                { return set_style_normal_bool_property("FauxItalic", v); }
	bool set_style_normal_horizontal_scale(const double v)                         { return set_style_normal_number_property("HorizontalScale", v); }
	bool set_style_normal_vertical_scale(const double v)                           { return set_style_normal_number_property("VerticalScale", v); }
	bool set_style_normal_tracking(const int32_t v)                                { return set_style_normal_int32_property("Tracking", v); }
	bool set_style_normal_auto_kerning(const bool v)                               { return set_style_normal_bool_property("AutoKerning", v); }
	bool set_style_normal_baseline_shift(const double v)                           { return set_style_normal_number_property("BaselineShift", v); }
	bool set_style_normal_font_caps(const TextLayerEnum::FontCaps v)                           { return set_style_normal_int32_property("FontCaps", static_cast<int32_t>(v)); }
	bool set_style_normal_font_baseline(const TextLayerEnum::FontBaseline v)                    { return set_style_normal_int32_property("FontBaseline", static_cast<int32_t>(v)); }
	bool set_style_normal_no_break(const bool v)                                   { return set_style_normal_bool_property("NoBreak", v); }
	bool set_style_normal_language(const int32_t v)                                { return set_style_normal_int32_property("Language", v); }
	bool set_style_normal_character_direction(const TextLayerEnum::CharacterDirection v) { return set_style_normal_int32_property("CharacterDirection", static_cast<int32_t>(v)); }
	bool set_style_normal_baseline_direction(const TextLayerEnum::BaselineDirection v)   { return set_style_normal_int32_property("BaselineDirection", static_cast<int32_t>(v)); }
	bool set_style_normal_tsume(const double v)                                    { return set_style_normal_number_property("Tsume", v); }
	bool set_style_normal_kashida(const int32_t v)                                 { return set_style_normal_int32_property("Kashida", v); }
	bool set_style_normal_diacritic_pos(const TextLayerEnum::DiacriticPosition v)       { return set_style_normal_int32_property("DiacriticPos", static_cast<int32_t>(v)); }
	bool set_style_normal_ligatures(const bool v)                                  { return set_style_normal_bool_property("Ligatures", v); }
	bool set_style_normal_dligatures(const bool v)                                 { return set_style_normal_bool_property("DLigatures", v); }
	bool set_style_normal_underline(const bool v)                                  { return set_style_normal_bool_property("Underline", v); }
	bool set_style_normal_strikethrough(const bool v)                              { return set_style_normal_bool_property("Strikethrough", v); }
	bool set_style_normal_stroke_flag(const bool v)                                { return set_style_normal_bool_property("StrokeFlag", v); }
	bool set_style_normal_fill_flag(const bool v)                                  { return set_style_normal_bool_property("FillFlag", v); }
	bool set_style_normal_fill_first(const bool v)                                 { return set_style_normal_bool_property("FillFirst", v); }
	bool set_style_normal_outline_width(const double v)                            { return set_style_normal_number_property("OutlineWidth", v); }
	bool set_style_normal_fill_color(const std::vector<double>& v)                 { return set_style_normal_color_values_property("FillColor", v); }
	bool set_style_normal_stroke_color(const std::vector<double>& v)               { return set_style_normal_color_values_property("StrokeColor", v); }

private:

	std::optional<int32_t> style_normal_int32_property(const std::string_view property_key) const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto style_data = TextLayerDetail::style_normal_sheet_data(parsed.root);
			if (style_data == nullptr) continue;
			const auto value = EngineData::find_dict_value(*style_data, property_key);
			int32_t parsed_value = 0;
			if (value == nullptr || !TextLayerDetail::number_value_to_int32(*value, parsed_value)) return std::nullopt;
			return parsed_value;
		}
		return std::nullopt;
	}

	std::optional<double> style_normal_number_property(const std::string_view property_key) const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto style_data = TextLayerDetail::style_normal_sheet_data(parsed.root);
			if (style_data == nullptr) continue;
			const auto value = EngineData::find_dict_value(*style_data, property_key);
			if (value == nullptr || value->type != EngineData::ValueType::Number) return std::nullopt;
			return value->number_value;
		}
		return std::nullopt;
	}

	std::optional<bool> style_normal_bool_property(const std::string_view property_key) const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto style_data = TextLayerDetail::style_normal_sheet_data(parsed.root);
			if (style_data == nullptr) continue;
			const auto value = EngineData::find_dict_value(*style_data, property_key);
			if (value == nullptr || value->type != EngineData::ValueType::Boolean) return std::nullopt;
			return value->bool_value;
		}
		return std::nullopt;
	}

	std::optional<std::vector<double>> style_normal_color_values_property(const std::string_view color_key) const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto style_data = TextLayerDetail::style_normal_sheet_data(parsed.root);
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

	bool set_style_normal_int32_property(const std::string_view property_key, const int32_t value)
	{
		return set_style_normal_number_property(property_key, static_cast<double>(value));
	}

	bool set_style_normal_number_property(const std::string_view property_key, const double value)
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
			auto style_data = TextLayerDetail::style_normal_sheet_data(parsed.root);
			if (style_data == nullptr) continue;
			auto property_value = EngineData::find_dict_value(*style_data, property_key);
			if (property_value == nullptr) return false;
			const size_t old_start = property_value->start_offset;
			const size_t old_end = property_value->end_offset;
			if (!EngineData::set_number(*property_value, value)) return false;
			auto new_bytes = EngineData::format_value_bytes(*property_value);
			EngineData::splice_payload(payload, old_start, old_end, new_bytes);
			return TextLayerDetail::write_engine_payload(*block, engine_span_opt.value(), payload);
		}
		return false;
	}

	bool set_style_normal_bool_property(const std::string_view property_key, const bool value)
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
			auto style_data = TextLayerDetail::style_normal_sheet_data(parsed.root);
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

	bool set_style_normal_color_values_property(const std::string_view color_key, const std::vector<double>& values)
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
			auto style_data = TextLayerDetail::style_normal_sheet_data(parsed.root);
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
