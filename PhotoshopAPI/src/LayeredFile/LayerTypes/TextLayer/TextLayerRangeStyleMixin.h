#pragma once

/// \file TextLayerRangeStyleMixin.h
///
/// Provides a high-level, range-based styling API on top of the low-level
/// per-run setters.  The user specifies character ranges (or text substrings)
/// and the mixin handles the run splitting and per-run property application
/// automatically.
///
/// Usage examples (C++):
///   layer->style_range(6, 10).set_bold(true);
///   layer->style_text("World").set_fill_color({1, 1, 0, 0}).set_font_size(32);
///   layer->style_all().set_font("TimesBold");
///   layer->paragraph_range(17, 31).set_justification(Justification::Center);
///
/// Multiple occurrences for style_text / paragraph_text:
///   style_text("the")        -> all occurrences  (occurrence == 0, default)
///   style_text("the", 1)     -> first occurrence  (1-based)
///   style_text("the", 2)     -> second occurrence
///   If the specified occurrence doesn't exist, the proxy is empty (no-op).

#include "Macros.h"
#include "TextLayerFwd.h"
#include "TextLayerEnum.h"

#include "Core/Struct/UnicodeString.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

PSAPI_NAMESPACE_BEGIN


template <typename Derived>
struct TextLayerRangeStyleMixin
{
protected:
	Derived* self() { return static_cast<Derived*>(this); }
	const Derived* self() const { return static_cast<const Derived*>(this); }

public:

	// =================================================================
	//  CharacterStyleRange — proxy for applying character styles
	//  to one or more character ranges.
	// =================================================================

	class CharacterStyleRange
	{
	public:
		CharacterStyleRange() = default;

		/// Construct with a back-pointer and a list of [start, end) ranges
		/// in UTF-16 code units.
		CharacterStyleRange(Derived* layer, std::vector<std::pair<size_t, size_t>> ranges)
			: layer_(layer), ranges_(std::move(ranges))
		{}

		/// \return true if this proxy targets at least one character range.
		bool valid() const { return layer_ != nullptr && !ranges_.empty(); }

		/// \return the number of character ranges this proxy covers.
		size_t range_count() const { return ranges_.size(); }

		// ── Character style setters (chainable) ─────────────────────

		CharacterStyleRange& set_font_size(double v)                                    { return apply_style([&](size_t r){ layer_->set_style_run_font_size(r, v); }); }
		CharacterStyleRange& set_leading(double v)                                      { return apply_style([&](size_t r){ layer_->set_style_run_leading(r, v); }); }
		CharacterStyleRange& set_auto_leading(bool v)                                   { return apply_style([&](size_t r){ layer_->set_style_run_auto_leading(r, v); }); }
		CharacterStyleRange& set_kerning(int32_t v)                                     { return apply_style([&](size_t r){ layer_->set_style_run_kerning(r, v); }); }
		CharacterStyleRange& set_bold(bool v)                                           { return apply_style([&](size_t r){ layer_->set_style_run_faux_bold(r, v); }); }
		CharacterStyleRange& set_italic(bool v)                                         { return apply_style([&](size_t r){ layer_->set_style_run_faux_italic(r, v); }); }
		CharacterStyleRange& set_underline(bool v)                                      { return apply_style([&](size_t r){ layer_->set_style_run_underline(r, v); }); }
		CharacterStyleRange& set_strikethrough(bool v)                                  { return apply_style([&](size_t r){ layer_->set_style_run_strikethrough(r, v); }); }
		CharacterStyleRange& set_fill_color(const std::vector<double>& v)               { return apply_style([&](size_t r){ layer_->set_style_run_fill_color(r, v); }); }
		CharacterStyleRange& set_stroke_color(const std::vector<double>& v)             { return apply_style([&](size_t r){ layer_->set_style_run_stroke_color(r, v); }); }
		CharacterStyleRange& set_stroke_flag(bool v)                                    { return apply_style([&](size_t r){ layer_->set_style_run_stroke_flag(r, v); }); }
		CharacterStyleRange& set_fill_flag(bool v)                                      { return apply_style([&](size_t r){ layer_->set_style_run_fill_flag(r, v); }); }
		CharacterStyleRange& set_fill_first(bool v)                                     { return apply_style([&](size_t r){ layer_->set_style_run_fill_first(r, v); }); }
		CharacterStyleRange& set_outline_width(double v)                                { return apply_style([&](size_t r){ layer_->set_style_run_outline_width(r, v); }); }
		CharacterStyleRange& set_horizontal_scale(double v)                             { return apply_style([&](size_t r){ layer_->set_style_run_horizontal_scale(r, v); }); }
		CharacterStyleRange& set_vertical_scale(double v)                               { return apply_style([&](size_t r){ layer_->set_style_run_vertical_scale(r, v); }); }
		CharacterStyleRange& set_tracking(int32_t v)                                    { return apply_style([&](size_t r){ layer_->set_style_run_tracking(r, v); }); }
		CharacterStyleRange& set_auto_kerning(bool v)                                   { return apply_style([&](size_t r){ layer_->set_style_run_auto_kerning(r, v); }); }
		CharacterStyleRange& set_baseline_shift(double v)                               { return apply_style([&](size_t r){ layer_->set_style_run_baseline_shift(r, v); }); }
		CharacterStyleRange& set_font_caps(TextLayerEnum::FontCaps v)                   { return apply_style([&](size_t r){ layer_->set_style_run_font_caps(r, v); }); }
		CharacterStyleRange& set_font_baseline(TextLayerEnum::FontBaseline v)           { return apply_style([&](size_t r){ layer_->set_style_run_font_baseline(r, v); }); }
		CharacterStyleRange& set_no_break(bool v)                                       { return apply_style([&](size_t r){ layer_->set_style_run_no_break(r, v); }); }
		CharacterStyleRange& set_language(int32_t v)                                    { return apply_style([&](size_t r){ layer_->set_style_run_language(r, v); }); }
		CharacterStyleRange& set_character_direction(TextLayerEnum::CharacterDirection v){ return apply_style([&](size_t r){ layer_->set_style_run_character_direction(r, v); }); }
		CharacterStyleRange& set_baseline_direction(TextLayerEnum::BaselineDirection v)  { return apply_style([&](size_t r){ layer_->set_style_run_baseline_direction(r, v); }); }
		CharacterStyleRange& set_tsume(double v)                                        { return apply_style([&](size_t r){ layer_->set_style_run_tsume(r, v); }); }
		CharacterStyleRange& set_kashida(int32_t v)                                     { return apply_style([&](size_t r){ layer_->set_style_run_kashida(r, v); }); }
		CharacterStyleRange& set_diacritic_pos(TextLayerEnum::DiacriticPosition v)      { return apply_style([&](size_t r){ layer_->set_style_run_diacritic_pos(r, v); }); }
		CharacterStyleRange& set_ligatures(bool v)                                      { return apply_style([&](size_t r){ layer_->set_style_run_ligatures(r, v); }); }
		CharacterStyleRange& set_dligatures(bool v)                                     { return apply_style([&](size_t r){ layer_->set_style_run_dligatures(r, v); }); }

		// ── Font management (by PostScript name) ────────────────────

		/// Set the font for all characters in this range by PostScript name.
		/// If the font isn't already in the FontSet, it is added automatically.
		CharacterStyleRange& set_font(const std::string& postscript_name)
		{
			return apply_style([&](size_t r) {
				layer_->set_style_run_font_by_name(r, postscript_name);
			});
		}

		/// Set the font by index (for callers who manage font indices directly).
		CharacterStyleRange& set_font_index(int32_t v)
		{
			return apply_style([&](size_t r) {
				layer_->set_style_run_font(r, v);
			});
		}

	private:
		Derived* layer_ = nullptr;
		std::vector<std::pair<size_t, size_t>> ranges_; // [start, end) in UTF-16 code units

		/// Core resolver: for each stored range, determine which runs are
		/// affected, split at boundaries if needed, then invoke the setter
		/// on every fully-covered run.
		template <typename Fn>
		CharacterStyleRange& apply_style(Fn&& setter)
		{
			if (!layer_) return *this;

			for (const auto& [start, end] : ranges_)
			{
				if (start >= end) continue;
				apply_to_style_runs(start, end, std::forward<Fn>(setter));
			}
			return *this;
		}

		/// Apply a setter to all style runs covered by [start, end).
		/// Splits runs at boundaries if start/end fall mid-run.
		template <typename Fn>
		void apply_to_style_runs(size_t start, size_t end, Fn&& setter)
		{
			// 1. Get current run lengths
			auto lengths_opt = layer_->style_run_lengths();
			if (!lengths_opt || lengths_opt->empty()) return;
			auto lengths = *lengths_opt;

			size_t total = 0;
			for (auto l : lengths) total += static_cast<size_t>(l);

			// Clamp
			if (start >= total) return;
			if (end > total) end = total;
			if (start >= end) return;

			// 2. Compute cumulative start offsets: [0, len0, len0+len1, ...]
			auto cumulative = compute_cumulative(lengths);

			// 3. Find first run containing 'start'
			size_t first_run = find_run_for_offset(cumulative, start);

			// 4. Split at 'start' if it falls mid-run
			if (cumulative[first_run] < start)
			{
				const size_t offset_in_run = start - cumulative[first_run];
				layer_->split_style_run(first_run, offset_in_run);
				++first_run; // the target content is now in the right half

				// Refresh
				lengths_opt = layer_->style_run_lengths();
				if (!lengths_opt || lengths_opt->empty()) return;
				lengths = *lengths_opt;
				cumulative = compute_cumulative(lengths);
			}

			// 5. Find last run containing 'end - 1'
			size_t last_run = find_run_for_offset(cumulative, end - 1);

			// 6. Split at 'end' if it falls mid-run
			if (cumulative[last_run + 1] > end)
			{
				const size_t offset_in_run = end - cumulative[last_run];
				layer_->split_style_run(last_run, offset_in_run);
				// last_run now points to the left part (which is fully inside our range)
			}

			// 7. Apply setter to all runs [first_run .. last_run]
			for (size_t r = first_run; r <= last_run; ++r)
			{
				setter(r);
			}
		}

		static std::vector<size_t> compute_cumulative(const std::vector<int32_t>& lengths)
		{
			std::vector<size_t> cum(lengths.size() + 1, 0u);
			for (size_t i = 0; i < lengths.size(); ++i)
				cum[i + 1] = cum[i] + static_cast<size_t>(lengths[i]);
			return cum;
		}

		/// Find the run index such that cumulative[r] <= offset < cumulative[r+1].
		static size_t find_run_for_offset(const std::vector<size_t>& cumulative, size_t offset)
		{
			for (size_t r = 0; r + 1 < cumulative.size(); ++r)
			{
				if (offset < cumulative[r + 1])
					return r;
			}
			return cumulative.size() >= 2 ? cumulative.size() - 2 : 0;
		}
	};


	// =================================================================
	//  ParagraphStyleRange — proxy for applying paragraph styles
	//  to one or more character ranges.
	// =================================================================

	class ParagraphStyleRange
	{
	public:
		ParagraphStyleRange() = default;

		ParagraphStyleRange(Derived* layer, std::vector<std::pair<size_t, size_t>> ranges)
			: layer_(layer), ranges_(std::move(ranges))
		{}

		bool valid() const { return layer_ != nullptr && !ranges_.empty(); }
		size_t range_count() const { return ranges_.size(); }

		// ── Paragraph style setters (chainable) ─────────────────────

		ParagraphStyleRange& set_justification(TextLayerEnum::Justification v)          { return apply_style([&](size_t r){ layer_->set_paragraph_run_justification(r, v); }); }
		ParagraphStyleRange& set_first_line_indent(double v)                            { return apply_style([&](size_t r){ layer_->set_paragraph_run_first_line_indent(r, v); }); }
		ParagraphStyleRange& set_start_indent(double v)                                 { return apply_style([&](size_t r){ layer_->set_paragraph_run_start_indent(r, v); }); }
		ParagraphStyleRange& set_end_indent(double v)                                   { return apply_style([&](size_t r){ layer_->set_paragraph_run_end_indent(r, v); }); }
		ParagraphStyleRange& set_space_before(double v)                                 { return apply_style([&](size_t r){ layer_->set_paragraph_run_space_before(r, v); }); }
		ParagraphStyleRange& set_space_after(double v)                                  { return apply_style([&](size_t r){ layer_->set_paragraph_run_space_after(r, v); }); }
		ParagraphStyleRange& set_auto_hyphenate(bool v)                                 { return apply_style([&](size_t r){ layer_->set_paragraph_run_auto_hyphenate(r, v); }); }
		ParagraphStyleRange& set_hyphenated_word_size(int32_t v)                        { return apply_style([&](size_t r){ layer_->set_paragraph_run_hyphenated_word_size(r, v); }); }
		ParagraphStyleRange& set_pre_hyphen(int32_t v)                                  { return apply_style([&](size_t r){ layer_->set_paragraph_run_pre_hyphen(r, v); }); }
		ParagraphStyleRange& set_post_hyphen(int32_t v)                                 { return apply_style([&](size_t r){ layer_->set_paragraph_run_post_hyphen(r, v); }); }
		ParagraphStyleRange& set_consecutive_hyphens(int32_t v)                         { return apply_style([&](size_t r){ layer_->set_paragraph_run_consecutive_hyphens(r, v); }); }
		ParagraphStyleRange& set_zone(double v)                                         { return apply_style([&](size_t r){ layer_->set_paragraph_run_zone(r, v); }); }
		ParagraphStyleRange& set_word_spacing(const std::vector<double>& v)             { return apply_style([&](size_t r){ layer_->set_paragraph_run_word_spacing(r, v); }); }
		ParagraphStyleRange& set_letter_spacing(const std::vector<double>& v)           { return apply_style([&](size_t r){ layer_->set_paragraph_run_letter_spacing(r, v); }); }
		ParagraphStyleRange& set_glyph_spacing(const std::vector<double>& v)            { return apply_style([&](size_t r){ layer_->set_paragraph_run_glyph_spacing(r, v); }); }
		ParagraphStyleRange& set_auto_leading(double v)                                 { return apply_style([&](size_t r){ layer_->set_paragraph_run_auto_leading(r, v); }); }
		ParagraphStyleRange& set_leading_type(TextLayerEnum::LeadingType v)             { return apply_style([&](size_t r){ layer_->set_paragraph_run_leading_type(r, v); }); }
		ParagraphStyleRange& set_hanging(bool v)                                        { return apply_style([&](size_t r){ layer_->set_paragraph_run_hanging(r, v); }); }
		ParagraphStyleRange& set_burasagari(bool v)                                     { return apply_style([&](size_t r){ layer_->set_paragraph_run_burasagari(r, v); }); }
		ParagraphStyleRange& set_kinsoku_order(TextLayerEnum::KinsokuOrder v)           { return apply_style([&](size_t r){ layer_->set_paragraph_run_kinsoku_order(r, v); }); }
		ParagraphStyleRange& set_every_line_composer(bool v)                            { return apply_style([&](size_t r){ layer_->set_paragraph_run_every_line_composer(r, v); }); }

	private:
		Derived* layer_ = nullptr;
		std::vector<std::pair<size_t, size_t>> ranges_;

		template <typename Fn>
		ParagraphStyleRange& apply_style(Fn&& setter)
		{
			if (!layer_) return *this;
			for (const auto& [start, end] : ranges_)
			{
				if (start >= end) continue;
				apply_to_paragraph_runs(start, end, std::forward<Fn>(setter));
			}
			return *this;
		}

		template <typename Fn>
		void apply_to_paragraph_runs(size_t start, size_t end, Fn&& setter)
		{
			auto lengths_opt = layer_->paragraph_run_lengths();
			if (!lengths_opt || lengths_opt->empty()) return;
			auto lengths = *lengths_opt;

			size_t total = 0;
			for (auto l : lengths) total += static_cast<size_t>(l);

			if (start >= total) return;
			if (end > total) end = total;
			if (start >= end) return;

			auto cumulative = compute_cumulative(lengths);

			size_t first_run = find_run_for_offset(cumulative, start);

			if (cumulative[first_run] < start)
			{
				const size_t offset_in_run = start - cumulative[first_run];
				layer_->split_paragraph_run(first_run, offset_in_run);
				++first_run;

				lengths_opt = layer_->paragraph_run_lengths();
				if (!lengths_opt || lengths_opt->empty()) return;
				lengths = *lengths_opt;
				cumulative = compute_cumulative(lengths);
			}

			size_t last_run = find_run_for_offset(cumulative, end - 1);

			if (cumulative[last_run + 1] > end)
			{
				const size_t offset_in_run = end - cumulative[last_run];
				layer_->split_paragraph_run(last_run, offset_in_run);
			}

			for (size_t r = first_run; r <= last_run; ++r)
			{
				setter(r);
			}
		}

		static std::vector<size_t> compute_cumulative(const std::vector<int32_t>& lengths)
		{
			std::vector<size_t> cum(lengths.size() + 1, 0u);
			for (size_t i = 0; i < lengths.size(); ++i)
				cum[i + 1] = cum[i] + static_cast<size_t>(lengths[i]);
			return cum;
		}

		static size_t find_run_for_offset(const std::vector<size_t>& cumulative, size_t offset)
		{
			for (size_t r = 0; r + 1 < cumulative.size(); ++r)
			{
				if (offset < cumulative[r + 1])
					return r;
			}
			return cumulative.size() >= 2 ? cumulative.size() - 2 : 0;
		}
	};


	// =================================================================
	//  Public factory methods
	// =================================================================

	/// Apply character styles to a range of characters [start, end).
	/// Positions are in UTF-16 code units (matches style_run_lengths()).
	CharacterStyleRange style_range(size_t start, size_t end)
	{
		return CharacterStyleRange(self(), { {start, end} });
	}

	/// Apply character styles to all occurrences of a text substring.
	///
	/// \param needle       The substring to search for (UTF-8).
	/// \param occurrence   0 = all occurrences (default), 1 = first, 2 = second, etc.
	CharacterStyleRange style_text(const std::string& needle, size_t occurrence = 0)
	{
		auto ranges = find_text_ranges(needle, occurrence);
		return CharacterStyleRange(self(), std::move(ranges));
	}

	/// Apply character styles to the entire text content.
	CharacterStyleRange style_all()
	{
		auto lengths_opt = self()->style_run_lengths();
		if (!lengths_opt || lengths_opt->empty())
			return CharacterStyleRange(self(), {});

		size_t total = 0;
		for (auto l : *lengths_opt) total += static_cast<size_t>(l);

		// Exclude trailing \r from the total so we don't style the
		// invisible trailing character
		return CharacterStyleRange(self(), { {0, total} });
	}

	/// Apply paragraph styles to a range of characters [start, end).
	ParagraphStyleRange paragraph_range(size_t start, size_t end)
	{
		return ParagraphStyleRange(self(), { {start, end} });
	}

	/// Apply paragraph styles to all occurrences of a text substring.
	///
	/// \param needle       The substring to search for (UTF-8).
	/// \param occurrence   0 = all occurrences (default), 1 = first, 2 = second, etc.
	ParagraphStyleRange paragraph_text(const std::string& needle, size_t occurrence = 0)
	{
		auto ranges = find_text_ranges(needle, occurrence);
		return ParagraphStyleRange(self(), std::move(ranges));
	}

	/// Apply paragraph styles to the entire text content.
	ParagraphStyleRange paragraph_all()
	{
		auto lengths_opt = self()->paragraph_run_lengths();
		if (!lengths_opt || lengths_opt->empty())
			return ParagraphStyleRange(self(), {});

		size_t total = 0;
		for (auto l : *lengths_opt) total += static_cast<size_t>(l);

		return ParagraphStyleRange(self(), { {0, total} });
	}

private:

	/// Find text substring occurrences, returning [start, end) ranges
	/// in UTF-16 code units.
	std::vector<std::pair<size_t, size_t>> find_text_ranges(
		const std::string& needle_utf8, size_t occurrence) const
	{
		std::vector<std::pair<size_t, size_t>> result;

		// Get the layer text as UTF-8
		auto text_opt = self()->text();
		if (!text_opt || text_opt->empty()) return result;
		const auto& text_utf8 = *text_opt;

		// Convert both text and needle to UTF-16
		const auto text_u16 = UnicodeString::convertUTF8ToUTF16LE(text_utf8);
		const auto needle_u16 = UnicodeString::convertUTF8ToUTF16LE(needle_utf8);
		if (needle_u16.empty()) return result;

		// Find all occurrences in UTF-16 space
		size_t found_count = 0;
		size_t pos = 0;
		while (pos + needle_u16.size() <= text_u16.size())
		{
			// Simple substring search in UTF-16
			bool match = true;
			for (size_t j = 0; j < needle_u16.size(); ++j)
			{
				if (text_u16[pos + j] != needle_u16[j])
				{
					match = false;
					break;
				}
			}

			if (match)
			{
				++found_count;
				if (occurrence == 0 || found_count == occurrence)
				{
					result.push_back({ pos, pos + needle_u16.size() });
					if (occurrence != 0)
						break; // found the specific occurrence we wanted
				}
				pos += needle_u16.size(); // advance past this match
			}
			else
			{
				++pos;
			}
		}

		return result;
	}
};


PSAPI_NAMESPACE_END
