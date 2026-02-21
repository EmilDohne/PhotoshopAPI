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
struct TextLayerParagraphNormalMixin
{
protected:
	Derived* self() { return static_cast<Derived*>(this); }
	const Derived* self() const { return static_cast<const Derived*>(this); }

public:

	/// Retrieve /TheNormalParagraphSheet index from EngineData /ResourceDict.
	std::optional<int32_t> paragraph_normal_sheet_index() const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			int32_t normal_index = 0;
			if (!TextLayerDetail::paragraph_normal_sheet_index_from_root(parsed.root, normal_index)) continue;
			return normal_index;
		}
		return std::nullopt;
	}

	// --- Paragraph normal getters ---
	std::optional<TextLayerEnum::Justification>          paragraph_normal_justification() const          { return TextLayerEnum::opt_enum<TextLayerEnum::Justification>(paragraph_normal_int32_property("Justification")); }
	std::optional<double>                    paragraph_normal_first_line_indent() const      { return paragraph_normal_number_property("FirstLineIndent"); }
	std::optional<double>                    paragraph_normal_start_indent() const           { return paragraph_normal_number_property("StartIndent"); }
	std::optional<double>                    paragraph_normal_end_indent() const             { return paragraph_normal_number_property("EndIndent"); }
	std::optional<double>                    paragraph_normal_space_before() const           { return paragraph_normal_number_property("SpaceBefore"); }
	std::optional<double>                    paragraph_normal_space_after() const            { return paragraph_normal_number_property("SpaceAfter"); }
	std::optional<bool>                      paragraph_normal_auto_hyphenate() const         { return paragraph_normal_bool_property("AutoHyphenate"); }
	std::optional<int32_t>                   paragraph_normal_hyphenated_word_size() const   { return paragraph_normal_int32_property("HyphenatedWordSize"); }
	std::optional<int32_t>                   paragraph_normal_pre_hyphen() const             { return paragraph_normal_int32_property("PreHyphen"); }
	std::optional<int32_t>                   paragraph_normal_post_hyphen() const            { return paragraph_normal_int32_property("PostHyphen"); }
	std::optional<int32_t>                   paragraph_normal_consecutive_hyphens() const    { return paragraph_normal_int32_property("ConsecutiveHyphens"); }
	std::optional<double>                    paragraph_normal_zone() const                   { return paragraph_normal_number_property("Zone"); }
	std::optional<std::vector<double>>       paragraph_normal_word_spacing() const           { return paragraph_normal_number_array_property("WordSpacing"); }
	std::optional<std::vector<double>>       paragraph_normal_letter_spacing() const         { return paragraph_normal_number_array_property("LetterSpacing"); }
	std::optional<std::vector<double>>       paragraph_normal_glyph_spacing() const          { return paragraph_normal_number_array_property("GlyphSpacing"); }
	std::optional<double>                    paragraph_normal_auto_leading() const           { return paragraph_normal_number_property("AutoLeading"); }
	std::optional<TextLayerEnum::LeadingType>            paragraph_normal_leading_type() const           { return TextLayerEnum::opt_enum<TextLayerEnum::LeadingType>(paragraph_normal_int32_property("LeadingType")); }
	std::optional<bool>                      paragraph_normal_hanging() const                { return paragraph_normal_bool_property("Hanging"); }
	std::optional<bool>                      paragraph_normal_burasagari() const             { return paragraph_normal_bool_property("Burasagari"); }
	std::optional<TextLayerEnum::KinsokuOrder>           paragraph_normal_kinsoku_order() const          { return TextLayerEnum::opt_enum<TextLayerEnum::KinsokuOrder>(paragraph_normal_int32_property("KinsokuOrder")); }
	std::optional<bool>                      paragraph_normal_every_line_composer() const    { return paragraph_normal_bool_property("EveryLineComposer"); }

	// --- Paragraph normal setters ---
	bool set_paragraph_normal_sheet_index(const int32_t sheet_index)
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
			auto sheet_set = EngineData::find_by_path(parsed.root, { "ResourceDict", "ParagraphSheetSet" });
			if (sheet_set == nullptr || sheet_set->type != EngineData::ValueType::Array) continue;
			if (static_cast<size_t>(sheet_index) >= sheet_set->array_items.size()) return false;
			auto normal_index_value = EngineData::find_by_path(parsed.root, { "ResourceDict", "TheNormalParagraphSheet" });
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

	bool set_paragraph_normal_justification(const TextLayerEnum::Justification v)                          { return set_paragraph_normal_int32_property("Justification", static_cast<int32_t>(v)); }
	bool set_paragraph_normal_first_line_indent(const double v)                            { return set_paragraph_normal_number_property("FirstLineIndent", v); }
	bool set_paragraph_normal_start_indent(const double v)                                 { return set_paragraph_normal_number_property("StartIndent", v); }
	bool set_paragraph_normal_end_indent(const double v)                                   { return set_paragraph_normal_number_property("EndIndent", v); }
	bool set_paragraph_normal_space_before(const double v)                                 { return set_paragraph_normal_number_property("SpaceBefore", v); }
	bool set_paragraph_normal_space_after(const double v)                                  { return set_paragraph_normal_number_property("SpaceAfter", v); }
	bool set_paragraph_normal_auto_hyphenate(const bool v)                                 { return set_paragraph_normal_bool_property("AutoHyphenate", v); }
	bool set_paragraph_normal_hyphenated_word_size(const int32_t v)                        { return set_paragraph_normal_int32_property("HyphenatedWordSize", v); }
	bool set_paragraph_normal_pre_hyphen(const int32_t v)                                  { return set_paragraph_normal_int32_property("PreHyphen", v); }
	bool set_paragraph_normal_post_hyphen(const int32_t v)                                 { return set_paragraph_normal_int32_property("PostHyphen", v); }
	bool set_paragraph_normal_consecutive_hyphens(const int32_t v)                         { return set_paragraph_normal_int32_property("ConsecutiveHyphens", v); }
	bool set_paragraph_normal_zone(const double v)                                         { return set_paragraph_normal_number_property("Zone", v); }
	bool set_paragraph_normal_word_spacing(const std::vector<double>& v)                   { return set_paragraph_normal_number_array_property("WordSpacing", v); }
	bool set_paragraph_normal_letter_spacing(const std::vector<double>& v)                 { return set_paragraph_normal_number_array_property("LetterSpacing", v); }
	bool set_paragraph_normal_glyph_spacing(const std::vector<double>& v)                  { return set_paragraph_normal_number_array_property("GlyphSpacing", v); }
	bool set_paragraph_normal_auto_leading(const double v)                                 { return set_paragraph_normal_number_property("AutoLeading", v); }
	bool set_paragraph_normal_leading_type(const TextLayerEnum::LeadingType v)                              { return set_paragraph_normal_int32_property("LeadingType", static_cast<int32_t>(v)); }
	bool set_paragraph_normal_hanging(const bool v)                                        { return set_paragraph_normal_bool_property("Hanging", v); }
	bool set_paragraph_normal_burasagari(const bool v)                                     { return set_paragraph_normal_bool_property("Burasagari", v); }
	bool set_paragraph_normal_kinsoku_order(const TextLayerEnum::KinsokuOrder v)                            { return set_paragraph_normal_int32_property("KinsokuOrder", static_cast<int32_t>(v)); }
	bool set_paragraph_normal_every_line_composer(const bool v)                            { return set_paragraph_normal_bool_property("EveryLineComposer", v); }

private:

	std::optional<int32_t> paragraph_normal_int32_property(const std::string_view property_key) const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto properties = TextLayerDetail::paragraph_normal_sheet_properties(parsed.root);
			if (properties == nullptr) continue;
			const auto value = EngineData::find_dict_value(*properties, property_key);
			int32_t parsed_value = 0;
			if (value == nullptr || !TextLayerDetail::number_value_to_int32(*value, parsed_value)) return std::nullopt;
			return parsed_value;
		}
		return std::nullopt;
	}

	std::optional<double> paragraph_normal_number_property(const std::string_view property_key) const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto properties = TextLayerDetail::paragraph_normal_sheet_properties(parsed.root);
			if (properties == nullptr) continue;
			const auto value = EngineData::find_dict_value(*properties, property_key);
			if (value == nullptr || value->type != EngineData::ValueType::Number) return std::nullopt;
			return value->number_value;
		}
		return std::nullopt;
	}

	std::optional<bool> paragraph_normal_bool_property(const std::string_view property_key) const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto properties = TextLayerDetail::paragraph_normal_sheet_properties(parsed.root);
			if (properties == nullptr) continue;
			const auto value = EngineData::find_dict_value(*properties, property_key);
			if (value == nullptr || value->type != EngineData::ValueType::Boolean) return std::nullopt;
			return value->bool_value;
		}
		return std::nullopt;
	}

	std::optional<std::vector<double>> paragraph_normal_number_array_property(const std::string_view property_key) const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto properties = TextLayerDetail::paragraph_normal_sheet_properties(parsed.root);
			if (properties == nullptr) continue;
			const auto value = EngineData::find_dict_value(*properties, property_key);
			if (value == nullptr) return std::nullopt;
			std::vector<double> out_values{};
			if (!EngineData::as_double_vector(*value, out_values)) return std::nullopt;
			return out_values;
		}
		return std::nullopt;
	}

	bool set_paragraph_normal_int32_property(const std::string_view property_key, const int32_t value)
	{
		return set_paragraph_normal_number_property(property_key, static_cast<double>(value));
	}

	bool set_paragraph_normal_number_property(const std::string_view property_key, const double value)
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
			auto properties = TextLayerDetail::paragraph_normal_sheet_properties(parsed.root);
			if (properties == nullptr) continue;
			auto property_value = EngineData::find_dict_value(*properties, property_key);
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

	bool set_paragraph_normal_bool_property(const std::string_view property_key, const bool value)
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
			auto properties = TextLayerDetail::paragraph_normal_sheet_properties(parsed.root);
			if (properties == nullptr) continue;
			auto property_value = EngineData::find_dict_value(*properties, property_key);
			if (property_value == nullptr || property_value->type != EngineData::ValueType::Boolean) return false;
			const size_t old_start = property_value->start_offset;
			const size_t old_end = property_value->end_offset;
			property_value->bool_value = value;
			auto new_bytes = EngineData::format_value_bytes(*property_value);
			EngineData::splice_payload(payload, old_start, old_end, new_bytes);
			return TextLayerDetail::write_engine_payload(*block, engine_span_opt.value(), payload);
		}
		return false;
	}

	bool set_paragraph_normal_number_array_property(const std::string_view property_key, const std::vector<double>& values)
	{
		if (values.empty()) return false;
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
			auto properties = TextLayerDetail::paragraph_normal_sheet_properties(parsed.root);
			if (properties == nullptr) continue;
			auto property_value = EngineData::find_dict_value(*properties, property_key);
			if (property_value == nullptr) return false;
			const size_t old_start = property_value->start_offset;
			const size_t old_end = property_value->end_offset;
			if (!EngineData::set_double_array(*property_value, values)) return false;
			for (auto& item : property_value->array_items) item.is_integer = false;
			auto new_bytes = EngineData::format_value_bytes(*property_value);
			EngineData::splice_payload(payload, old_start, old_end, new_bytes);
			return TextLayerDetail::write_engine_payload(*block, engine_span_opt.value(), payload);
		}
		return false;
	}
};

PSAPI_NAMESPACE_END
