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
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

PSAPI_NAMESPACE_BEGIN

template <typename Derived>
struct TextLayerParagraphRunMixin
{
protected:
	Derived* self() { return static_cast<Derived*>(this); }
	const Derived* self() const { return static_cast<const Derived*>(this); }

public:

	size_t paragraph_run_count() const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto run_array = EngineData::find_by_path(parsed.root, { "EngineDict", "ParagraphRun", "RunArray" });
			if (run_array != nullptr && run_array->type == EngineData::ValueType::Array)
				return run_array->array_items.size();
		}
		return 0u;
	}

	size_t paragraph_sheet_count() const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto sheet_set = EngineData::find_by_path(parsed.root, { "ResourceDict", "ParagraphSheetSet" });
			if (sheet_set != nullptr && sheet_set->type == EngineData::ValueType::Array)
				return sheet_set->array_items.size();
		}
		return 0u;
	}

	// --- Paragraph run getters ---
	std::optional<TextLayerEnum::Justification>          paragraph_run_justification(const size_t i) const          { return TextLayerEnum::opt_enum<TextLayerEnum::Justification>(paragraph_run_int32_property(i, "Justification")); }
	std::optional<double>                    paragraph_run_first_line_indent(const size_t i) const      { return paragraph_run_number_property(i, "FirstLineIndent"); }
	std::optional<double>                    paragraph_run_start_indent(const size_t i) const           { return paragraph_run_number_property(i, "StartIndent"); }
	std::optional<double>                    paragraph_run_end_indent(const size_t i) const             { return paragraph_run_number_property(i, "EndIndent"); }
	std::optional<double>                    paragraph_run_space_before(const size_t i) const           { return paragraph_run_number_property(i, "SpaceBefore"); }
	std::optional<double>                    paragraph_run_space_after(const size_t i) const            { return paragraph_run_number_property(i, "SpaceAfter"); }
	std::optional<bool>                      paragraph_run_auto_hyphenate(const size_t i) const         { return paragraph_run_bool_property(i, "AutoHyphenate"); }
	std::optional<int32_t>                   paragraph_run_hyphenated_word_size(const size_t i) const   { return paragraph_run_int32_property(i, "HyphenatedWordSize"); }
	std::optional<int32_t>                   paragraph_run_pre_hyphen(const size_t i) const             { return paragraph_run_int32_property(i, "PreHyphen"); }
	std::optional<int32_t>                   paragraph_run_post_hyphen(const size_t i) const            { return paragraph_run_int32_property(i, "PostHyphen"); }
	std::optional<int32_t>                   paragraph_run_consecutive_hyphens(const size_t i) const    { return paragraph_run_int32_property(i, "ConsecutiveHyphens"); }
	std::optional<double>                    paragraph_run_zone(const size_t i) const                   { return paragraph_run_number_property(i, "Zone"); }
	std::optional<std::vector<double>>       paragraph_run_word_spacing(const size_t i) const           { return paragraph_run_number_array_property(i, "WordSpacing"); }
	std::optional<std::vector<double>>       paragraph_run_letter_spacing(const size_t i) const         { return paragraph_run_number_array_property(i, "LetterSpacing"); }
	std::optional<std::vector<double>>       paragraph_run_glyph_spacing(const size_t i) const          { return paragraph_run_number_array_property(i, "GlyphSpacing"); }
	std::optional<double>                    paragraph_run_auto_leading(const size_t i) const           { return paragraph_run_number_property(i, "AutoLeading"); }
	std::optional<TextLayerEnum::LeadingType>            paragraph_run_leading_type(const size_t i) const           { return TextLayerEnum::opt_enum<TextLayerEnum::LeadingType>(paragraph_run_int32_property(i, "LeadingType")); }
	std::optional<bool>                      paragraph_run_hanging(const size_t i) const                { return paragraph_run_bool_property(i, "Hanging"); }
	std::optional<bool>                      paragraph_run_burasagari(const size_t i) const             { return paragraph_run_bool_property(i, "Burasagari"); }
	std::optional<TextLayerEnum::KinsokuOrder>           paragraph_run_kinsoku_order(const size_t i) const          { return TextLayerEnum::opt_enum<TextLayerEnum::KinsokuOrder>(paragraph_run_int32_property(i, "KinsokuOrder")); }
	std::optional<bool>                      paragraph_run_every_line_composer(const size_t i) const    { return paragraph_run_bool_property(i, "EveryLineComposer"); }

	// --- Paragraph run setters ---
	void set_paragraph_run_justification(const size_t i, const TextLayerEnum::Justification v)                  { throw_on_set_failure(set_paragraph_run_int32_property(i, "Justification", static_cast<int32_t>(v)), "set_paragraph_run_justification"); }
	void set_paragraph_run_first_line_indent(const size_t i, const double v)                            { throw_on_set_failure(set_paragraph_run_number_property(i, "FirstLineIndent", v), "set_paragraph_run_first_line_indent"); }
	void set_paragraph_run_start_indent(const size_t i, const double v)                                 { throw_on_set_failure(set_paragraph_run_number_property(i, "StartIndent", v), "set_paragraph_run_start_indent"); }
	void set_paragraph_run_end_indent(const size_t i, const double v)                                   { throw_on_set_failure(set_paragraph_run_number_property(i, "EndIndent", v), "set_paragraph_run_end_indent"); }
	void set_paragraph_run_space_before(const size_t i, const double v)                                 { throw_on_set_failure(set_paragraph_run_number_property(i, "SpaceBefore", v), "set_paragraph_run_space_before"); }
	void set_paragraph_run_space_after(const size_t i, const double v)                                  { throw_on_set_failure(set_paragraph_run_number_property(i, "SpaceAfter", v), "set_paragraph_run_space_after"); }
	void set_paragraph_run_auto_hyphenate(const size_t i, const bool v)                                 { throw_on_set_failure(set_paragraph_run_bool_property(i, "AutoHyphenate", v), "set_paragraph_run_auto_hyphenate"); }
	void set_paragraph_run_hyphenated_word_size(const size_t i, const int32_t v)                        { throw_on_set_failure(set_paragraph_run_int32_property(i, "HyphenatedWordSize", v), "set_paragraph_run_hyphenated_word_size"); }
	void set_paragraph_run_pre_hyphen(const size_t i, const int32_t v)                                  { throw_on_set_failure(set_paragraph_run_int32_property(i, "PreHyphen", v), "set_paragraph_run_pre_hyphen"); }
	void set_paragraph_run_post_hyphen(const size_t i, const int32_t v)                                 { throw_on_set_failure(set_paragraph_run_int32_property(i, "PostHyphen", v), "set_paragraph_run_post_hyphen"); }
	void set_paragraph_run_consecutive_hyphens(const size_t i, const int32_t v)                         { throw_on_set_failure(set_paragraph_run_int32_property(i, "ConsecutiveHyphens", v), "set_paragraph_run_consecutive_hyphens"); }
	void set_paragraph_run_zone(const size_t i, const double v)                                         { throw_on_set_failure(set_paragraph_run_number_property(i, "Zone", v), "set_paragraph_run_zone"); }
	void set_paragraph_run_word_spacing(const size_t i, const std::vector<double>& v)                   { throw_on_set_failure(set_paragraph_run_number_array_property(i, "WordSpacing", v), "set_paragraph_run_word_spacing"); }
	void set_paragraph_run_letter_spacing(const size_t i, const std::vector<double>& v)                 { throw_on_set_failure(set_paragraph_run_number_array_property(i, "LetterSpacing", v), "set_paragraph_run_letter_spacing"); }
	void set_paragraph_run_glyph_spacing(const size_t i, const std::vector<double>& v)                  { throw_on_set_failure(set_paragraph_run_number_array_property(i, "GlyphSpacing", v), "set_paragraph_run_glyph_spacing"); }
	void set_paragraph_run_auto_leading(const size_t i, const double v)                                 { throw_on_set_failure(set_paragraph_run_number_property(i, "AutoLeading", v), "set_paragraph_run_auto_leading"); }
	void set_paragraph_run_leading_type(const size_t i, const TextLayerEnum::LeadingType v)                  { throw_on_set_failure(set_paragraph_run_int32_property(i, "LeadingType", static_cast<int32_t>(v)), "set_paragraph_run_leading_type"); }
	void set_paragraph_run_hanging(const size_t i, const bool v)                                        { throw_on_set_failure(set_paragraph_run_bool_property(i, "Hanging", v), "set_paragraph_run_hanging"); }
	void set_paragraph_run_burasagari(const size_t i, const bool v)                                     { throw_on_set_failure(set_paragraph_run_bool_property(i, "Burasagari", v), "set_paragraph_run_burasagari"); }
	void set_paragraph_run_kinsoku_order(const size_t i, const TextLayerEnum::KinsokuOrder v)                { throw_on_set_failure(set_paragraph_run_int32_property(i, "KinsokuOrder", static_cast<int32_t>(v)), "set_paragraph_run_kinsoku_order"); }
	void set_paragraph_run_every_line_composer(const size_t i, const bool v)                            { throw_on_set_failure(set_paragraph_run_bool_property(i, "EveryLineComposer", v), "set_paragraph_run_every_line_composer"); }

private:
	void throw_on_set_failure(const bool ok, const char* method_name) const
	{
		if (!ok)
		{
			throw std::invalid_argument(std::string("TextLayer::") + method_name + "() failed");
		}
	}

	std::optional<double> paragraph_run_number_property(const size_t run_index, const std::string_view property_key) const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto properties = TextLayerDetail::paragraph_run_properties_for_run(parsed.root, run_index);
			if (properties == nullptr) continue;
			const auto value = EngineData::find_dict_value(*properties, property_key);
			if (value == nullptr || value->type != EngineData::ValueType::Number) return std::nullopt;
			return value->number_value;
		}
		return std::nullopt;
	}

	std::optional<int32_t> paragraph_run_int32_property(const size_t run_index, const std::string_view property_key) const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto properties = TextLayerDetail::paragraph_run_properties_for_run(parsed.root, run_index);
			if (properties == nullptr) continue;
			const auto value = EngineData::find_dict_value(*properties, property_key);
			int32_t parsed_value = 0;
			if (value == nullptr || !TextLayerDetail::number_value_to_int32(*value, parsed_value)) return std::nullopt;
			return parsed_value;
		}
		return std::nullopt;
	}

	std::optional<bool> paragraph_run_bool_property(const size_t run_index, const std::string_view property_key) const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto properties = TextLayerDetail::paragraph_run_properties_for_run(parsed.root, run_index);
			if (properties == nullptr) continue;
			const auto value = EngineData::find_dict_value(*properties, property_key);
			if (value == nullptr || value->type != EngineData::ValueType::Boolean) return std::nullopt;
			return value->bool_value;
		}
		return std::nullopt;
	}

	std::optional<std::vector<double>> paragraph_run_number_array_property(const size_t run_index, const std::string_view property_key) const
	{
		for (const auto& block : self()->text_tagged_blocks())
		{
			const auto payload = TextLayerDetail::read_engine_payload(*block);
			if (!payload.has_value()) continue;
			auto parsed = EngineData::parse(*payload);
			if (!parsed.ok) continue;
			const auto properties = TextLayerDetail::paragraph_run_properties_for_run(parsed.root, run_index);
			if (properties == nullptr) continue;
			const auto value = EngineData::find_dict_value(*properties, property_key);
			if (value == nullptr) return std::nullopt;
			std::vector<double> out_values{};
			if (!EngineData::as_double_vector(*value, out_values)) return std::nullopt;
			return out_values;
		}
		return std::nullopt;
	}

	bool set_paragraph_run_number_property(const size_t run_index, const std::string_view property_key, const double value)
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
			auto properties = TextLayerDetail::paragraph_run_properties_for_run(parsed.root, run_index);
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

	bool set_paragraph_run_int32_property(const size_t run_index, const std::string_view property_key, const int32_t value)
	{
		return set_paragraph_run_number_property(run_index, property_key, static_cast<double>(value));
	}

	bool set_paragraph_run_bool_property(const size_t run_index, const std::string_view property_key, const bool value)
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
			auto properties = TextLayerDetail::paragraph_run_properties_for_run(parsed.root, run_index);
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

	bool set_paragraph_run_number_array_property(const size_t run_index, const std::string_view property_key, const std::vector<double>& values)
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
			auto properties = TextLayerDetail::paragraph_run_properties_for_run(parsed.root, run_index);
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
