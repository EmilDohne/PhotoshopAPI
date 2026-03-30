#pragma once

#include "Macros.h"
#include "TextLayerFwd.h"
#include "TextLayerEnum.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

PSAPI_NAMESPACE_BEGIN

/// \brief CRTP mixin providing proxy accessor objects for TextLayer.
///
/// The proxy structs are lightweight non-owning wrappers that provide a
/// grouped, ergonomic interface on top of the flat functions from the
/// other mixins.  They are intended to be used as temporaries and must
/// not outlive the parent TextLayer.
template <typename Derived>
struct TextLayerProxiesMixin
{
protected:
	Derived* self() { return static_cast<Derived*>(this); }
	const Derived* self() const { return static_cast<const Derived*>(this); }

public:

	// ===================================================================
	//  Proxy structs
	// ===================================================================

	/// Proxy for a single character style run (indexed).
	struct StyleRunProxy
	{
		Derived* layer;
		size_t index;

		// --- Getters ---
		std::optional<double>                    font_size()              { return layer->style_run_font_size(index); }
		std::optional<double>                    leading()                { return layer->style_run_leading(index); }
		std::optional<bool>                      auto_leading()           { return layer->style_run_auto_leading(index); }
		std::optional<int32_t>                   kerning()                { return layer->style_run_kerning(index); }
		std::optional<std::vector<double>>       fill_color()             { return layer->style_run_fill_color(index); }
		std::optional<std::vector<double>>       stroke_color()           { return layer->style_run_stroke_color(index); }
		std::optional<int32_t>                   font()                   { return layer->style_run_font(index); }
		std::optional<bool>                      faux_bold()              { return layer->style_run_faux_bold(index); }
		std::optional<bool>                      faux_italic()            { return layer->style_run_faux_italic(index); }
		std::optional<double>                    horizontal_scale()       { return layer->style_run_horizontal_scale(index); }
		std::optional<double>                    vertical_scale()         { return layer->style_run_vertical_scale(index); }
		std::optional<int32_t>                   tracking()               { return layer->style_run_tracking(index); }
		std::optional<bool>                      auto_kerning()           { return layer->style_run_auto_kerning(index); }
		std::optional<double>                    baseline_shift()         { return layer->style_run_baseline_shift(index); }
		std::optional<TextLayerEnum::FontCaps>              font_caps()              { return layer->style_run_font_caps(index); }
		std::optional<TextLayerEnum::FontBaseline>           font_baseline()          { return layer->style_run_font_baseline(index); }
		std::optional<bool>                      no_break()               { return layer->style_run_no_break(index); }
		std::optional<int32_t>                   language()               { return layer->style_run_language(index); }
		std::optional<TextLayerEnum::CharacterDirection>    character_direction()    { return layer->style_run_character_direction(index); }
		std::optional<TextLayerEnum::BaselineDirection>     baseline_direction()     { return layer->style_run_baseline_direction(index); }
		std::optional<double>                    tsume()                  { return layer->style_run_tsume(index); }
		std::optional<int32_t>                   kashida()                { return layer->style_run_kashida(index); }
		std::optional<TextLayerEnum::DiacriticPosition>     diacritic_pos()          { return layer->style_run_diacritic_pos(index); }
		std::optional<bool>                      ligatures()              { return layer->style_run_ligatures(index); }
		std::optional<bool>                      dligatures()             { return layer->style_run_dligatures(index); }
		std::optional<bool>                      underline()              { return layer->style_run_underline(index); }
		std::optional<bool>                      strikethrough()          { return layer->style_run_strikethrough(index); }
		std::optional<bool>                      stroke_flag()            { return layer->style_run_stroke_flag(index); }
		std::optional<bool>                      fill_flag()              { return layer->style_run_fill_flag(index); }
		std::optional<bool>                      fill_first()             { return layer->style_run_fill_first(index); }
		std::optional<double>                    outline_width()          { return layer->style_run_outline_width(index); }

		// --- Setters ---
		void set_font_size(const double v) { layer->set_style_run_font_size(index, v); }
		void set_leading(const double v) { layer->set_style_run_leading(index, v); }
		void set_auto_leading(const bool v) { layer->set_style_run_auto_leading(index, v); }
		void set_kerning(const int32_t v) { layer->set_style_run_kerning(index, v); }
		void set_fill_color(const std::vector<double>& v) { layer->set_style_run_fill_color(index, v); }
		void set_stroke_color(const std::vector<double>& v) { layer->set_style_run_stroke_color(index, v); }
		void set_font(const int32_t v) { layer->set_style_run_font(index, v); }
		void set_font_by_name(const std::string& v) { layer->set_style_run_font_by_name(index, v); }
		void set_faux_bold(const bool v) { layer->set_style_run_faux_bold(index, v); }
		void set_faux_italic(const bool v) { layer->set_style_run_faux_italic(index, v); }
		void set_horizontal_scale(const double v) { layer->set_style_run_horizontal_scale(index, v); }
		void set_vertical_scale(const double v) { layer->set_style_run_vertical_scale(index, v); }
		void set_tracking(const int32_t v) { layer->set_style_run_tracking(index, v); }
		void set_auto_kerning(const bool v) { layer->set_style_run_auto_kerning(index, v); }
		void set_baseline_shift(const double v) { layer->set_style_run_baseline_shift(index, v); }
		void set_font_caps(const TextLayerEnum::FontCaps v) { layer->set_style_run_font_caps(index, v); }
		void set_font_baseline(const TextLayerEnum::FontBaseline v) { layer->set_style_run_font_baseline(index, v); }
		void set_no_break(const bool v) { layer->set_style_run_no_break(index, v); }
		void set_language(const int32_t v) { layer->set_style_run_language(index, v); }
		void set_character_direction(const TextLayerEnum::CharacterDirection v) { layer->set_style_run_character_direction(index, v); }
		void set_baseline_direction(const TextLayerEnum::BaselineDirection v) { layer->set_style_run_baseline_direction(index, v); }
		void set_tsume(const double v) { layer->set_style_run_tsume(index, v); }
		void set_kashida(const int32_t v) { layer->set_style_run_kashida(index, v); }
		void set_diacritic_pos(const TextLayerEnum::DiacriticPosition v) { layer->set_style_run_diacritic_pos(index, v); }
		void set_ligatures(const bool v) { layer->set_style_run_ligatures(index, v); }
		void set_dligatures(const bool v) { layer->set_style_run_dligatures(index, v); }
		void set_underline(const bool v) { layer->set_style_run_underline(index, v); }
		void set_strikethrough(const bool v) { layer->set_style_run_strikethrough(index, v); }
		void set_stroke_flag(const bool v) { layer->set_style_run_stroke_flag(index, v); }
		void set_fill_flag(const bool v) { layer->set_style_run_fill_flag(index, v); }
		void set_fill_first(const bool v) { layer->set_style_run_fill_first(index, v); }
		void set_outline_width(const double v) { layer->set_style_run_outline_width(index, v); }
	};

	/// Proxy for the normal (default) character style sheet (no index).
	struct StyleNormalProxy
	{
		Derived* layer;

		// --- Getters ---
		std::optional<int32_t>                   sheet_index()            { return layer->style_normal_sheet_index(); }
		std::optional<int32_t>                   font()                   { return layer->style_normal_font(); }
		std::optional<double>                    font_size()              { return layer->style_normal_font_size(); }
		std::optional<double>                    leading()                { return layer->style_normal_leading(); }
		std::optional<bool>                      auto_leading()           { return layer->style_normal_auto_leading(); }
		std::optional<int32_t>                   kerning()                { return layer->style_normal_kerning(); }
		std::optional<bool>                      faux_bold()              { return layer->style_normal_faux_bold(); }
		std::optional<bool>                      faux_italic()            { return layer->style_normal_faux_italic(); }
		std::optional<double>                    horizontal_scale()       { return layer->style_normal_horizontal_scale(); }
		std::optional<double>                    vertical_scale()         { return layer->style_normal_vertical_scale(); }
		std::optional<int32_t>                   tracking()               { return layer->style_normal_tracking(); }
		std::optional<bool>                      auto_kerning()           { return layer->style_normal_auto_kerning(); }
		std::optional<double>                    baseline_shift()         { return layer->style_normal_baseline_shift(); }
		std::optional<TextLayerEnum::FontCaps>              font_caps()              { return layer->style_normal_font_caps(); }
		std::optional<TextLayerEnum::FontBaseline>           font_baseline()          { return layer->style_normal_font_baseline(); }
		std::optional<bool>                      no_break()               { return layer->style_normal_no_break(); }
		std::optional<int32_t>                   language()               { return layer->style_normal_language(); }
		std::optional<TextLayerEnum::CharacterDirection>    character_direction()    { return layer->style_normal_character_direction(); }
		std::optional<TextLayerEnum::BaselineDirection>     baseline_direction()     { return layer->style_normal_baseline_direction(); }
		std::optional<double>                    tsume()                  { return layer->style_normal_tsume(); }
		std::optional<int32_t>                   kashida()                { return layer->style_normal_kashida(); }
		std::optional<TextLayerEnum::DiacriticPosition>     diacritic_pos()          { return layer->style_normal_diacritic_pos(); }
		std::optional<bool>                      ligatures()              { return layer->style_normal_ligatures(); }
		std::optional<bool>                      dligatures()             { return layer->style_normal_dligatures(); }
		std::optional<bool>                      underline()              { return layer->style_normal_underline(); }
		std::optional<bool>                      strikethrough()          { return layer->style_normal_strikethrough(); }
		std::optional<bool>                      stroke_flag()            { return layer->style_normal_stroke_flag(); }
		std::optional<bool>                      fill_flag()              { return layer->style_normal_fill_flag(); }
		std::optional<bool>                      fill_first()             { return layer->style_normal_fill_first(); }
		std::optional<double>                    outline_width()          { return layer->style_normal_outline_width(); }
		std::optional<std::vector<double>>       fill_color()             { return layer->style_normal_fill_color(); }
		std::optional<std::vector<double>>       stroke_color()           { return layer->style_normal_stroke_color(); }

		// --- Setters ---
		void set_sheet_index(const int32_t v) { layer->set_style_normal_sheet_index(v); }
		void set_font(const int32_t v) { layer->set_style_normal_font(v); }
		void set_font_by_name(const std::string& v) { layer->set_style_normal_font_by_name(v); }
		void set_font_size(const double v) { layer->set_style_normal_font_size(v); }
		void set_leading(const double v) { layer->set_style_normal_leading(v); }
		void set_auto_leading(const bool v) { layer->set_style_normal_auto_leading(v); }
		void set_kerning(const int32_t v) { layer->set_style_normal_kerning(v); }
		void set_faux_bold(const bool v) { layer->set_style_normal_faux_bold(v); }
		void set_faux_italic(const bool v) { layer->set_style_normal_faux_italic(v); }
		void set_horizontal_scale(const double v) { layer->set_style_normal_horizontal_scale(v); }
		void set_vertical_scale(const double v) { layer->set_style_normal_vertical_scale(v); }
		void set_tracking(const int32_t v) { layer->set_style_normal_tracking(v); }
		void set_auto_kerning(const bool v) { layer->set_style_normal_auto_kerning(v); }
		void set_baseline_shift(const double v) { layer->set_style_normal_baseline_shift(v); }
		void set_font_caps(const TextLayerEnum::FontCaps v) { layer->set_style_normal_font_caps(v); }
		void set_font_baseline(const TextLayerEnum::FontBaseline v) { layer->set_style_normal_font_baseline(v); }
		void set_no_break(const bool v) { layer->set_style_normal_no_break(v); }
		void set_language(const int32_t v) { layer->set_style_normal_language(v); }
		void set_character_direction(const TextLayerEnum::CharacterDirection v) { layer->set_style_normal_character_direction(v); }
		void set_baseline_direction(const TextLayerEnum::BaselineDirection v) { layer->set_style_normal_baseline_direction(v); }
		void set_tsume(const double v) { layer->set_style_normal_tsume(v); }
		void set_kashida(const int32_t v) { layer->set_style_normal_kashida(v); }
		void set_diacritic_pos(const TextLayerEnum::DiacriticPosition v) { layer->set_style_normal_diacritic_pos(v); }
		void set_ligatures(const bool v) { layer->set_style_normal_ligatures(v); }
		void set_dligatures(const bool v) { layer->set_style_normal_dligatures(v); }
		void set_underline(const bool v) { layer->set_style_normal_underline(v); }
		void set_strikethrough(const bool v) { layer->set_style_normal_strikethrough(v); }
		void set_stroke_flag(const bool v) { layer->set_style_normal_stroke_flag(v); }
		void set_fill_flag(const bool v) { layer->set_style_normal_fill_flag(v); }
		void set_fill_first(const bool v) { layer->set_style_normal_fill_first(v); }
		void set_outline_width(const double v) { layer->set_style_normal_outline_width(v); }
		void set_fill_color(const std::vector<double>& v) { layer->set_style_normal_fill_color(v); }
		void set_stroke_color(const std::vector<double>& v) { layer->set_style_normal_stroke_color(v); }
	};

	/// Proxy for a single paragraph run (indexed).
	struct ParagraphRunProxy
	{
		Derived* layer;
		size_t index;

		// --- Getters ---
		std::optional<TextLayerEnum::Justification> justification()       { return layer->paragraph_run_justification(index); }
		std::optional<double>                    first_line_indent()      { return layer->paragraph_run_first_line_indent(index); }
		std::optional<double>                    start_indent()           { return layer->paragraph_run_start_indent(index); }
		std::optional<double>                    end_indent()             { return layer->paragraph_run_end_indent(index); }
		std::optional<double>                    space_before()           { return layer->paragraph_run_space_before(index); }
		std::optional<double>                    space_after()            { return layer->paragraph_run_space_after(index); }
		std::optional<bool>                      auto_hyphenate()         { return layer->paragraph_run_auto_hyphenate(index); }
		std::optional<int32_t>                   hyphenated_word_size()   { return layer->paragraph_run_hyphenated_word_size(index); }
		std::optional<int32_t>                   pre_hyphen()             { return layer->paragraph_run_pre_hyphen(index); }
		std::optional<int32_t>                   post_hyphen()            { return layer->paragraph_run_post_hyphen(index); }
		std::optional<int32_t>                   consecutive_hyphens()    { return layer->paragraph_run_consecutive_hyphens(index); }
		std::optional<double>                    zone()                   { return layer->paragraph_run_zone(index); }
		std::optional<std::vector<double>>       word_spacing()           { return layer->paragraph_run_word_spacing(index); }
		std::optional<std::vector<double>>       letter_spacing()         { return layer->paragraph_run_letter_spacing(index); }
		std::optional<std::vector<double>>       glyph_spacing()          { return layer->paragraph_run_glyph_spacing(index); }
		std::optional<double>                              auto_leading()           { return layer->paragraph_run_auto_leading(index); }
		std::optional<TextLayerEnum::LeadingType>          leading_type()           { return layer->paragraph_run_leading_type(index); }
		std::optional<bool>                                hanging()                { return layer->paragraph_run_hanging(index); }
		std::optional<bool>                                burasagari()             { return layer->paragraph_run_burasagari(index); }
		std::optional<TextLayerEnum::KinsokuOrder>         kinsoku_order()          { return layer->paragraph_run_kinsoku_order(index); }
		std::optional<bool>                      every_line_composer()    { return layer->paragraph_run_every_line_composer(index); }

		// --- Setters ---
		void set_justification(const TextLayerEnum::Justification v) { layer->set_paragraph_run_justification(index, v); }
		void set_first_line_indent(const double v) { layer->set_paragraph_run_first_line_indent(index, v); }
		void set_start_indent(const double v) { layer->set_paragraph_run_start_indent(index, v); }
		void set_end_indent(const double v) { layer->set_paragraph_run_end_indent(index, v); }
		void set_space_before(const double v) { layer->set_paragraph_run_space_before(index, v); }
		void set_space_after(const double v) { layer->set_paragraph_run_space_after(index, v); }
		void set_auto_hyphenate(const bool v) { layer->set_paragraph_run_auto_hyphenate(index, v); }
		void set_hyphenated_word_size(const int32_t v) { layer->set_paragraph_run_hyphenated_word_size(index, v); }
		void set_pre_hyphen(const int32_t v) { layer->set_paragraph_run_pre_hyphen(index, v); }
		void set_post_hyphen(const int32_t v) { layer->set_paragraph_run_post_hyphen(index, v); }
		void set_consecutive_hyphens(const int32_t v) { layer->set_paragraph_run_consecutive_hyphens(index, v); }
		void set_zone(const double v) { layer->set_paragraph_run_zone(index, v); }
		void set_word_spacing(const std::vector<double>& v) { layer->set_paragraph_run_word_spacing(index, v); }
		void set_letter_spacing(const std::vector<double>& v) { layer->set_paragraph_run_letter_spacing(index, v); }
		void set_glyph_spacing(const std::vector<double>& v) { layer->set_paragraph_run_glyph_spacing(index, v); }
		void set_auto_leading(const double v) { layer->set_paragraph_run_auto_leading(index, v); }
		void set_leading_type(const TextLayerEnum::LeadingType v) { layer->set_paragraph_run_leading_type(index, v); }
		void set_hanging(const bool v) { layer->set_paragraph_run_hanging(index, v); }
		void set_burasagari(const bool v) { layer->set_paragraph_run_burasagari(index, v); }
		void set_kinsoku_order(const TextLayerEnum::KinsokuOrder v) { layer->set_paragraph_run_kinsoku_order(index, v); }
		void set_every_line_composer(const bool v) { layer->set_paragraph_run_every_line_composer(index, v); }
	};

	/// Proxy for the normal (default) paragraph sheet (no index).
	struct ParagraphNormalProxy
	{
		Derived* layer;

		// --- Getters ---
		std::optional<int32_t>                   sheet_index()            { return layer->paragraph_normal_sheet_index(); }
		std::optional<TextLayerEnum::Justification> justification()       { return layer->paragraph_normal_justification(); }
		std::optional<double>                    first_line_indent()      { return layer->paragraph_normal_first_line_indent(); }
		std::optional<double>                    start_indent()           { return layer->paragraph_normal_start_indent(); }
		std::optional<double>                    end_indent()             { return layer->paragraph_normal_end_indent(); }
		std::optional<double>                    space_before()           { return layer->paragraph_normal_space_before(); }
		std::optional<double>                    space_after()            { return layer->paragraph_normal_space_after(); }
		std::optional<bool>                      auto_hyphenate()         { return layer->paragraph_normal_auto_hyphenate(); }
		std::optional<int32_t>                   hyphenated_word_size()   { return layer->paragraph_normal_hyphenated_word_size(); }
		std::optional<int32_t>                   pre_hyphen()             { return layer->paragraph_normal_pre_hyphen(); }
		std::optional<int32_t>                   post_hyphen()            { return layer->paragraph_normal_post_hyphen(); }
		std::optional<int32_t>                   consecutive_hyphens()    { return layer->paragraph_normal_consecutive_hyphens(); }
		std::optional<double>                    zone()                   { return layer->paragraph_normal_zone(); }
		std::optional<std::vector<double>>       word_spacing()           { return layer->paragraph_normal_word_spacing(); }
		std::optional<std::vector<double>>       letter_spacing()         { return layer->paragraph_normal_letter_spacing(); }
		std::optional<std::vector<double>>       glyph_spacing()          { return layer->paragraph_normal_glyph_spacing(); }
		std::optional<double>                              auto_leading()           { return layer->paragraph_normal_auto_leading(); }
		std::optional<TextLayerEnum::LeadingType>          leading_type()           { return layer->paragraph_normal_leading_type(); }
		std::optional<bool>                                hanging()                { return layer->paragraph_normal_hanging(); }
		std::optional<bool>                                burasagari()             { return layer->paragraph_normal_burasagari(); }
		std::optional<TextLayerEnum::KinsokuOrder>         kinsoku_order()          { return layer->paragraph_normal_kinsoku_order(); }
		std::optional<bool>                      every_line_composer()    { return layer->paragraph_normal_every_line_composer(); }

		// --- Setters ---
		void set_sheet_index(const int32_t v) { layer->set_paragraph_normal_sheet_index(v); }
		void set_justification(const TextLayerEnum::Justification v) { layer->set_paragraph_normal_justification(v); }
		void set_first_line_indent(const double v) { layer->set_paragraph_normal_first_line_indent(v); }
		void set_start_indent(const double v) { layer->set_paragraph_normal_start_indent(v); }
		void set_end_indent(const double v) { layer->set_paragraph_normal_end_indent(v); }
		void set_space_before(const double v) { layer->set_paragraph_normal_space_before(v); }
		void set_space_after(const double v) { layer->set_paragraph_normal_space_after(v); }
		void set_auto_hyphenate(const bool v) { layer->set_paragraph_normal_auto_hyphenate(v); }
		void set_hyphenated_word_size(const int32_t v) { layer->set_paragraph_normal_hyphenated_word_size(v); }
		void set_pre_hyphen(const int32_t v) { layer->set_paragraph_normal_pre_hyphen(v); }
		void set_post_hyphen(const int32_t v) { layer->set_paragraph_normal_post_hyphen(v); }
		void set_consecutive_hyphens(const int32_t v) { layer->set_paragraph_normal_consecutive_hyphens(v); }
		void set_zone(const double v) { layer->set_paragraph_normal_zone(v); }
		void set_word_spacing(const std::vector<double>& v) { layer->set_paragraph_normal_word_spacing(v); }
		void set_letter_spacing(const std::vector<double>& v) { layer->set_paragraph_normal_letter_spacing(v); }
		void set_glyph_spacing(const std::vector<double>& v) { layer->set_paragraph_normal_glyph_spacing(v); }
		void set_auto_leading(const double v) { layer->set_paragraph_normal_auto_leading(v); }
		void set_leading_type(const TextLayerEnum::LeadingType v) { layer->set_paragraph_normal_leading_type(v); }
		void set_hanging(const bool v) { layer->set_paragraph_normal_hanging(v); }
		void set_burasagari(const bool v) { layer->set_paragraph_normal_burasagari(v); }
		void set_kinsoku_order(const TextLayerEnum::KinsokuOrder v) { layer->set_paragraph_normal_kinsoku_order(v); }
		void set_every_line_composer(const bool v) { layer->set_paragraph_normal_every_line_composer(v); }
	};

	/// Proxy for a single font entry in the FontSet (indexed).
	struct FontProxy
	{
		Derived* layer;
		size_t index;

		// --- Getters ---
		std::optional<std::string>  postscript_name()  { return layer->font_postscript_name(index); }
		std::optional<std::string>  name()             { return layer->font_name(index); }
		std::optional<TextLayerEnum::FontScript>  script()  { return layer->font_script(index); }
		std::optional<TextLayerEnum::FontType>    type()    { return layer->font_type(index); }
		std::optional<int32_t>      synthetic()        { return layer->font_synthetic(index); }
		bool                        is_sentinel()      { return layer->is_sentinel_font(index); }

		// --- Setters ---
		void set_postscript_name(const std::string& v) { layer->set_font_postscript_name(index, v); }
	};

	/// Proxy for the FontSet as a whole (no index).
	struct FontSetProxy
	{
		Derived* layer;

		// --- Getters ---
		size_t                      count()                               { return layer->font_count(); }
		std::vector<size_t>         used_indices()                        { return layer->used_font_indices(); }
		std::vector<std::string>    used_names()                          { return layer->used_font_names(); }
		int32_t                     find_index(const std::string& name)   { return layer->find_font_index(name); }

		// --- Mutators ---
		int32_t add(const std::string& postscript_name,
		            const TextLayerEnum::FontType font_type_val = TextLayerEnum::FontType::OpenType,
		            const TextLayerEnum::FontScript script = TextLayerEnum::FontScript::Roman,
		            const int32_t synthetic = 0)
		{
			return layer->add_font(postscript_name, font_type_val, script, synthetic);
		}
	};

	// ===================================================================
	//  Proxy accessor methods
	// ===================================================================

	/// Return a proxy for the style run at the given index.
	StyleRunProxy       style_run(const size_t run_index)   { return { self(), run_index }; }

	/// Return a proxy for the normal (default) character style sheet.
	StyleNormalProxy    style_normal()                       { return { self() }; }

	/// Return a proxy for the paragraph run at the given index.
	ParagraphRunProxy   paragraph_run(const size_t run_index) { return { self(), run_index }; }

	/// Return a proxy for the normal (default) paragraph sheet.
	ParagraphNormalProxy paragraph_normal()                  { return { self() }; }

	/// Return a proxy for the font at the given FontSet index.
	FontProxy           font(const size_t font_index)        { return { self(), font_index }; }

	/// Return a proxy for the FontSet as a whole.
	FontSetProxy        font_set()                           { return { self() }; }
};

PSAPI_NAMESPACE_END

