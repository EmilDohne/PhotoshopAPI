#pragma once

#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"
#include "LayeredFile/LayerTypes/Layer.h"
#include "LayeredFile/LayerTypes/TextLayer/TextLayerEnum.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>
#include <string>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


// ---------------------------------------------------------------------------
//  Proxy class declarations
// ---------------------------------------------------------------------------
//
// Each proxy type is a lightweight non-owning handle returned by the
// corresponding accessor on TextLayer (e.g. layer.style_run(0)).
// Proxies must not outlive the parent TextLayer.
// ---------------------------------------------------------------------------

template <typename T>
void declare_text_layer_proxies(py::module& m, const std::string& extension)
{
    using Layer = TextLayer<T>;

    // -----------------------------------------------------------------------
    //  StyleRunProxy
    // -----------------------------------------------------------------------
    using SRP = typename Layer::StyleRunProxy;
    py::class_<SRP>(m, ("StyleRunProxy" + extension).c_str())
        .def_property_readonly("font_size",           [](SRP& p){ return p.font_size(); })
        .def_property_readonly("leading",             [](SRP& p){ return p.leading(); })
        .def_property_readonly("auto_leading",        [](SRP& p){ return p.auto_leading(); })
        .def_property_readonly("kerning",             [](SRP& p){ return p.kerning(); })
        .def_property_readonly("fill_color",          [](SRP& p){ return p.fill_color(); })
        .def_property_readonly("stroke_color",        [](SRP& p){ return p.stroke_color(); })
        .def_property_readonly("font",                [](SRP& p){ return p.font(); })
        .def_property_readonly("faux_bold",           [](SRP& p){ return p.faux_bold(); })
        .def_property_readonly("faux_italic",         [](SRP& p){ return p.faux_italic(); })
        .def_property_readonly("horizontal_scale",    [](SRP& p){ return p.horizontal_scale(); })
        .def_property_readonly("vertical_scale",      [](SRP& p){ return p.vertical_scale(); })
        .def_property_readonly("tracking",            [](SRP& p){ return p.tracking(); })
        .def_property_readonly("auto_kerning",        [](SRP& p){ return p.auto_kerning(); })
        .def_property_readonly("baseline_shift",      [](SRP& p){ return p.baseline_shift(); })
        .def_property_readonly("font_caps",           [](SRP& p){ return p.font_caps(); })
        .def_property_readonly("font_baseline",       [](SRP& p){ return p.font_baseline(); })
        .def_property_readonly("no_break",            [](SRP& p){ return p.no_break(); })
        .def_property_readonly("language",            [](SRP& p){ return p.language(); })
        .def_property_readonly("character_direction", [](SRP& p){ return p.character_direction(); })
        .def_property_readonly("baseline_direction",  [](SRP& p){ return p.baseline_direction(); })
        .def_property_readonly("tsume",               [](SRP& p){ return p.tsume(); })
        .def_property_readonly("kashida",             [](SRP& p){ return p.kashida(); })
        .def_property_readonly("diacritic_pos",       [](SRP& p){ return p.diacritic_pos(); })
        .def_property_readonly("ligatures",           [](SRP& p){ return p.ligatures(); })
        .def_property_readonly("dligatures",          [](SRP& p){ return p.dligatures(); })
        .def_property_readonly("underline",           [](SRP& p){ return p.underline(); })
        .def_property_readonly("strikethrough",       [](SRP& p){ return p.strikethrough(); })
        .def_property_readonly("stroke_flag",         [](SRP& p){ return p.stroke_flag(); })
        .def_property_readonly("fill_flag",           [](SRP& p){ return p.fill_flag(); })
        .def_property_readonly("fill_first",          [](SRP& p){ return p.fill_first(); })
        .def_property_readonly("outline_width",       [](SRP& p){ return p.outline_width(); })
        .def("set_font_size",           [](SRP& p, double v)                         { return p.set_font_size(v); },           py::arg("font_size"))
        .def("set_leading",             [](SRP& p, double v)                         { return p.set_leading(v); },             py::arg("leading"))
        .def("set_auto_leading",        [](SRP& p, bool v)                           { return p.set_auto_leading(v); },        py::arg("auto_leading"))
        .def("set_kerning",             [](SRP& p, int32_t v)                        { return p.set_kerning(v); },             py::arg("kerning"))
        .def("set_fill_color",          [](SRP& p, const std::vector<double>& v)     { return p.set_fill_color(v); },          py::arg("values"))
        .def("set_stroke_color",        [](SRP& p, const std::vector<double>& v)     { return p.set_stroke_color(v); },        py::arg("values"))
        .def("set_font",                [](SRP& p, int32_t v)                        { return p.set_font(v); },                py::arg("font"))
        .def("set_font_by_name",        [](SRP& p, const std::string& v)             { return p.set_font_by_name(v); },        py::arg("postscript_name"))
        .def("set_faux_bold",           [](SRP& p, bool v)                           { return p.set_faux_bold(v); },           py::arg("faux_bold"))
        .def("set_faux_italic",         [](SRP& p, bool v)                           { return p.set_faux_italic(v); },         py::arg("faux_italic"))
        .def("set_horizontal_scale",    [](SRP& p, double v)                         { return p.set_horizontal_scale(v); },    py::arg("horizontal_scale"))
        .def("set_vertical_scale",      [](SRP& p, double v)                         { return p.set_vertical_scale(v); },      py::arg("vertical_scale"))
        .def("set_tracking",            [](SRP& p, int32_t v)                        { return p.set_tracking(v); },            py::arg("tracking"))
        .def("set_auto_kerning",        [](SRP& p, bool v)                           { return p.set_auto_kerning(v); },        py::arg("auto_kerning"))
        .def("set_baseline_shift",      [](SRP& p, double v)                         { return p.set_baseline_shift(v); },      py::arg("baseline_shift"))
        .def("set_font_caps",           [](SRP& p, TextLayerEnum::FontCaps v)             { return p.set_font_caps(v); },           py::arg("font_caps"))
        .def("set_font_baseline",       [](SRP& p, TextLayerEnum::FontBaseline v)          { return p.set_font_baseline(v); },       py::arg("font_baseline"))
        .def("set_no_break",            [](SRP& p, bool v)                           { return p.set_no_break(v); },            py::arg("no_break"))
        .def("set_language",            [](SRP& p, int32_t v)                        { return p.set_language(v); },            py::arg("language"))
        .def("set_character_direction", [](SRP& p, TextLayerEnum::CharacterDirection v)    { return p.set_character_direction(v); }, py::arg("character_direction"))
        .def("set_baseline_direction",  [](SRP& p, TextLayerEnum::BaselineDirection v)     { return p.set_baseline_direction(v); },  py::arg("baseline_direction"))
        .def("set_tsume",               [](SRP& p, double v)                         { return p.set_tsume(v); },               py::arg("tsume"))
        .def("set_kashida",             [](SRP& p, int32_t v)                        { return p.set_kashida(v); },             py::arg("kashida"))
        .def("set_diacritic_pos",       [](SRP& p, TextLayerEnum::DiacriticPosition v)    { return p.set_diacritic_pos(v); },       py::arg("diacritic_pos"))
        .def("set_ligatures",           [](SRP& p, bool v)                           { return p.set_ligatures(v); },           py::arg("ligatures"))
        .def("set_dligatures",          [](SRP& p, bool v)                           { return p.set_dligatures(v); },          py::arg("dligatures"))
        .def("set_underline",           [](SRP& p, bool v)                           { return p.set_underline(v); },           py::arg("underline"))
        .def("set_strikethrough",       [](SRP& p, bool v)                           { return p.set_strikethrough(v); },       py::arg("strikethrough"))
        .def("set_stroke_flag",         [](SRP& p, bool v)                           { return p.set_stroke_flag(v); },         py::arg("stroke_flag"))
        .def("set_fill_flag",           [](SRP& p, bool v)                           { return p.set_fill_flag(v); },           py::arg("fill_flag"))
        .def("set_fill_first",          [](SRP& p, bool v)                           { return p.set_fill_first(v); },          py::arg("fill_first"))
        .def("set_outline_width",       [](SRP& p, double v)                         { return p.set_outline_width(v); },       py::arg("outline_width"))
        ;

    // -----------------------------------------------------------------------
    //  StyleNormalProxy
    // -----------------------------------------------------------------------
    using SNP = typename Layer::StyleNormalProxy;
    py::class_<SNP>(m, ("StyleNormalProxy" + extension).c_str())
        .def_property_readonly("sheet_index",         [](SNP& p){ return p.sheet_index(); })
        .def_property_readonly("font",                [](SNP& p){ return p.font(); })
        .def_property_readonly("font_size",           [](SNP& p){ return p.font_size(); })
        .def_property_readonly("leading",             [](SNP& p){ return p.leading(); })
        .def_property_readonly("auto_leading",        [](SNP& p){ return p.auto_leading(); })
        .def_property_readonly("kerning",             [](SNP& p){ return p.kerning(); })
        .def_property_readonly("faux_bold",           [](SNP& p){ return p.faux_bold(); })
        .def_property_readonly("faux_italic",         [](SNP& p){ return p.faux_italic(); })
        .def_property_readonly("horizontal_scale",    [](SNP& p){ return p.horizontal_scale(); })
        .def_property_readonly("vertical_scale",      [](SNP& p){ return p.vertical_scale(); })
        .def_property_readonly("tracking",            [](SNP& p){ return p.tracking(); })
        .def_property_readonly("auto_kerning",        [](SNP& p){ return p.auto_kerning(); })
        .def_property_readonly("baseline_shift",      [](SNP& p){ return p.baseline_shift(); })
        .def_property_readonly("font_caps",           [](SNP& p){ return p.font_caps(); })
        .def_property_readonly("font_baseline",       [](SNP& p){ return p.font_baseline(); })
        .def_property_readonly("no_break",            [](SNP& p){ return p.no_break(); })
        .def_property_readonly("language",            [](SNP& p){ return p.language(); })
        .def_property_readonly("character_direction", [](SNP& p){ return p.character_direction(); })
        .def_property_readonly("baseline_direction",  [](SNP& p){ return p.baseline_direction(); })
        .def_property_readonly("tsume",               [](SNP& p){ return p.tsume(); })
        .def_property_readonly("kashida",             [](SNP& p){ return p.kashida(); })
        .def_property_readonly("diacritic_pos",       [](SNP& p){ return p.diacritic_pos(); })
        .def_property_readonly("ligatures",           [](SNP& p){ return p.ligatures(); })
        .def_property_readonly("dligatures",          [](SNP& p){ return p.dligatures(); })
        .def_property_readonly("underline",           [](SNP& p){ return p.underline(); })
        .def_property_readonly("strikethrough",       [](SNP& p){ return p.strikethrough(); })
        .def_property_readonly("stroke_flag",         [](SNP& p){ return p.stroke_flag(); })
        .def_property_readonly("fill_flag",           [](SNP& p){ return p.fill_flag(); })
        .def_property_readonly("fill_first",          [](SNP& p){ return p.fill_first(); })
        .def_property_readonly("outline_width",       [](SNP& p){ return p.outline_width(); })
        .def_property_readonly("fill_color",          [](SNP& p){ return p.fill_color(); })
        .def_property_readonly("stroke_color",        [](SNP& p){ return p.stroke_color(); })
        .def("set_sheet_index",         [](SNP& p, int32_t v)                        { return p.set_sheet_index(v); },         py::arg("sheet_index"))
        .def("set_font",                [](SNP& p, int32_t v)                        { return p.set_font(v); },                py::arg("font"))
        .def("set_font_by_name",        [](SNP& p, const std::string& v)             { return p.set_font_by_name(v); },        py::arg("postscript_name"))
        .def("set_font_size",           [](SNP& p, double v)                         { return p.set_font_size(v); },           py::arg("font_size"))
        .def("set_leading",             [](SNP& p, double v)                         { return p.set_leading(v); },             py::arg("leading"))
        .def("set_auto_leading",        [](SNP& p, bool v)                           { return p.set_auto_leading(v); },        py::arg("auto_leading"))
        .def("set_kerning",             [](SNP& p, int32_t v)                        { return p.set_kerning(v); },             py::arg("kerning"))
        .def("set_faux_bold",           [](SNP& p, bool v)                           { return p.set_faux_bold(v); },           py::arg("faux_bold"))
        .def("set_faux_italic",         [](SNP& p, bool v)                           { return p.set_faux_italic(v); },         py::arg("faux_italic"))
        .def("set_horizontal_scale",    [](SNP& p, double v)                         { return p.set_horizontal_scale(v); },    py::arg("horizontal_scale"))
        .def("set_vertical_scale",      [](SNP& p, double v)                         { return p.set_vertical_scale(v); },      py::arg("vertical_scale"))
        .def("set_tracking",            [](SNP& p, int32_t v)                        { return p.set_tracking(v); },            py::arg("tracking"))
        .def("set_auto_kerning",        [](SNP& p, bool v)                           { return p.set_auto_kerning(v); },        py::arg("auto_kerning"))
        .def("set_baseline_shift",      [](SNP& p, double v)                         { return p.set_baseline_shift(v); },      py::arg("baseline_shift"))
        .def("set_font_caps",           [](SNP& p, TextLayerEnum::FontCaps v)             { return p.set_font_caps(v); },           py::arg("font_caps"))
        .def("set_font_baseline",       [](SNP& p, TextLayerEnum::FontBaseline v)          { return p.set_font_baseline(v); },       py::arg("font_baseline"))
        .def("set_no_break",            [](SNP& p, bool v)                           { return p.set_no_break(v); },            py::arg("no_break"))
        .def("set_language",            [](SNP& p, int32_t v)                        { return p.set_language(v); },            py::arg("language"))
        .def("set_character_direction", [](SNP& p, TextLayerEnum::CharacterDirection v)    { return p.set_character_direction(v); }, py::arg("character_direction"))
        .def("set_baseline_direction",  [](SNP& p, TextLayerEnum::BaselineDirection v)     { return p.set_baseline_direction(v); },  py::arg("baseline_direction"))
        .def("set_tsume",               [](SNP& p, double v)                         { return p.set_tsume(v); },               py::arg("tsume"))
        .def("set_kashida",             [](SNP& p, int32_t v)                        { return p.set_kashida(v); },             py::arg("kashida"))
        .def("set_diacritic_pos",       [](SNP& p, TextLayerEnum::DiacriticPosition v)    { return p.set_diacritic_pos(v); },       py::arg("diacritic_pos"))
        .def("set_ligatures",           [](SNP& p, bool v)                           { return p.set_ligatures(v); },           py::arg("ligatures"))
        .def("set_dligatures",          [](SNP& p, bool v)                           { return p.set_dligatures(v); },          py::arg("dligatures"))
        .def("set_underline",           [](SNP& p, bool v)                           { return p.set_underline(v); },           py::arg("underline"))
        .def("set_strikethrough",       [](SNP& p, bool v)                           { return p.set_strikethrough(v); },       py::arg("strikethrough"))
        .def("set_stroke_flag",         [](SNP& p, bool v)                           { return p.set_stroke_flag(v); },         py::arg("stroke_flag"))
        .def("set_fill_flag",           [](SNP& p, bool v)                           { return p.set_fill_flag(v); },           py::arg("fill_flag"))
        .def("set_fill_first",          [](SNP& p, bool v)                           { return p.set_fill_first(v); },          py::arg("fill_first"))
        .def("set_outline_width",       [](SNP& p, double v)                         { return p.set_outline_width(v); },       py::arg("outline_width"))
        .def("set_fill_color",          [](SNP& p, const std::vector<double>& v)     { return p.set_fill_color(v); },          py::arg("values"))
        .def("set_stroke_color",        [](SNP& p, const std::vector<double>& v)     { return p.set_stroke_color(v); },        py::arg("values"))
        ;

    // -----------------------------------------------------------------------
    //  ParagraphRunProxy
    // -----------------------------------------------------------------------
    using PRP = typename Layer::ParagraphRunProxy;
    py::class_<PRP>(m, ("ParagraphRunProxy" + extension).c_str())
        .def_property_readonly("justification",        [](PRP& p){ return p.justification(); })
        .def_property_readonly("first_line_indent",    [](PRP& p){ return p.first_line_indent(); })
        .def_property_readonly("start_indent",         [](PRP& p){ return p.start_indent(); })
        .def_property_readonly("end_indent",           [](PRP& p){ return p.end_indent(); })
        .def_property_readonly("space_before",         [](PRP& p){ return p.space_before(); })
        .def_property_readonly("space_after",          [](PRP& p){ return p.space_after(); })
        .def_property_readonly("auto_hyphenate",       [](PRP& p){ return p.auto_hyphenate(); })
        .def_property_readonly("hyphenated_word_size", [](PRP& p){ return p.hyphenated_word_size(); })
        .def_property_readonly("pre_hyphen",           [](PRP& p){ return p.pre_hyphen(); })
        .def_property_readonly("post_hyphen",          [](PRP& p){ return p.post_hyphen(); })
        .def_property_readonly("consecutive_hyphens",  [](PRP& p){ return p.consecutive_hyphens(); })
        .def_property_readonly("zone",                 [](PRP& p){ return p.zone(); })
        .def_property_readonly("word_spacing",         [](PRP& p){ return p.word_spacing(); })
        .def_property_readonly("letter_spacing",       [](PRP& p){ return p.letter_spacing(); })
        .def_property_readonly("glyph_spacing",        [](PRP& p){ return p.glyph_spacing(); })
        .def_property_readonly("auto_leading",         [](PRP& p){ return p.auto_leading(); })
        .def_property_readonly("leading_type",         [](PRP& p){ return p.leading_type(); })
        .def_property_readonly("hanging",              [](PRP& p){ return p.hanging(); })
        .def_property_readonly("burasagari",           [](PRP& p){ return p.burasagari(); })
        .def_property_readonly("kinsoku_order",        [](PRP& p){ return p.kinsoku_order(); })
        .def_property_readonly("every_line_composer",  [](PRP& p){ return p.every_line_composer(); })
        .def("set_justification",        [](PRP& p, TextLayerEnum::Justification v)        { return p.set_justification(v); },        py::arg("justification"))
        .def("set_first_line_indent",    [](PRP& p, double v)                        { return p.set_first_line_indent(v); },    py::arg("first_line_indent"))
        .def("set_start_indent",         [](PRP& p, double v)                        { return p.set_start_indent(v); },         py::arg("start_indent"))
        .def("set_end_indent",           [](PRP& p, double v)                        { return p.set_end_indent(v); },           py::arg("end_indent"))
        .def("set_space_before",         [](PRP& p, double v)                        { return p.set_space_before(v); },         py::arg("space_before"))
        .def("set_space_after",          [](PRP& p, double v)                        { return p.set_space_after(v); },          py::arg("space_after"))
        .def("set_auto_hyphenate",       [](PRP& p, bool v)                          { return p.set_auto_hyphenate(v); },       py::arg("auto_hyphenate"))
        .def("set_hyphenated_word_size", [](PRP& p, int32_t v)                       { return p.set_hyphenated_word_size(v); }, py::arg("hyphenated_word_size"))
        .def("set_pre_hyphen",           [](PRP& p, int32_t v)                       { return p.set_pre_hyphen(v); },           py::arg("pre_hyphen"))
        .def("set_post_hyphen",          [](PRP& p, int32_t v)                       { return p.set_post_hyphen(v); },          py::arg("post_hyphen"))
        .def("set_consecutive_hyphens",  [](PRP& p, int32_t v)                       { return p.set_consecutive_hyphens(v); },  py::arg("consecutive_hyphens"))
        .def("set_zone",                 [](PRP& p, double v)                        { return p.set_zone(v); },                 py::arg("zone"))
        .def("set_word_spacing",         [](PRP& p, const std::vector<double>& v)    { return p.set_word_spacing(v); },         py::arg("word_spacing"))
        .def("set_letter_spacing",       [](PRP& p, const std::vector<double>& v)    { return p.set_letter_spacing(v); },       py::arg("letter_spacing"))
        .def("set_glyph_spacing",        [](PRP& p, const std::vector<double>& v)    { return p.set_glyph_spacing(v); },        py::arg("glyph_spacing"))
        .def("set_auto_leading",         [](PRP& p, double v)                        { return p.set_auto_leading(v); },         py::arg("auto_leading"))
        .def("set_leading_type",         [](PRP& p, TextLayerEnum::LeadingType v)          { return p.set_leading_type(v); },         py::arg("leading_type"))
        .def("set_hanging",              [](PRP& p, bool v)                          { return p.set_hanging(v); },              py::arg("hanging"))
        .def("set_burasagari",           [](PRP& p, bool v)                          { return p.set_burasagari(v); },           py::arg("burasagari"))
        .def("set_kinsoku_order",        [](PRP& p, TextLayerEnum::KinsokuOrder v)         { return p.set_kinsoku_order(v); },        py::arg("kinsoku_order"))
        .def("set_every_line_composer",  [](PRP& p, bool v)                          { return p.set_every_line_composer(v); },  py::arg("every_line_composer"))
        ;

    // -----------------------------------------------------------------------
    //  ParagraphNormalProxy
    // -----------------------------------------------------------------------
    using PNP = typename Layer::ParagraphNormalProxy;
    py::class_<PNP>(m, ("ParagraphNormalProxy" + extension).c_str())
        .def_property_readonly("sheet_index",          [](PNP& p){ return p.sheet_index(); })
        .def_property_readonly("justification",        [](PNP& p){ return p.justification(); })
        .def_property_readonly("first_line_indent",    [](PNP& p){ return p.first_line_indent(); })
        .def_property_readonly("start_indent",         [](PNP& p){ return p.start_indent(); })
        .def_property_readonly("end_indent",           [](PNP& p){ return p.end_indent(); })
        .def_property_readonly("space_before",         [](PNP& p){ return p.space_before(); })
        .def_property_readonly("space_after",          [](PNP& p){ return p.space_after(); })
        .def_property_readonly("auto_hyphenate",       [](PNP& p){ return p.auto_hyphenate(); })
        .def_property_readonly("hyphenated_word_size", [](PNP& p){ return p.hyphenated_word_size(); })
        .def_property_readonly("pre_hyphen",           [](PNP& p){ return p.pre_hyphen(); })
        .def_property_readonly("post_hyphen",          [](PNP& p){ return p.post_hyphen(); })
        .def_property_readonly("consecutive_hyphens",  [](PNP& p){ return p.consecutive_hyphens(); })
        .def_property_readonly("zone",                 [](PNP& p){ return p.zone(); })
        .def_property_readonly("word_spacing",         [](PNP& p){ return p.word_spacing(); })
        .def_property_readonly("letter_spacing",       [](PNP& p){ return p.letter_spacing(); })
        .def_property_readonly("glyph_spacing",        [](PNP& p){ return p.glyph_spacing(); })
        .def_property_readonly("auto_leading",         [](PNP& p){ return p.auto_leading(); })
        .def_property_readonly("leading_type",         [](PNP& p){ return p.leading_type(); })
        .def_property_readonly("hanging",              [](PNP& p){ return p.hanging(); })
        .def_property_readonly("burasagari",           [](PNP& p){ return p.burasagari(); })
        .def_property_readonly("kinsoku_order",        [](PNP& p){ return p.kinsoku_order(); })
        .def_property_readonly("every_line_composer",  [](PNP& p){ return p.every_line_composer(); })
        .def("set_sheet_index",          [](PNP& p, int32_t v)                       { return p.set_sheet_index(v); },          py::arg("sheet_index"))
        .def("set_justification",        [](PNP& p, TextLayerEnum::Justification v)        { return p.set_justification(v); },        py::arg("justification"))
        .def("set_first_line_indent",    [](PNP& p, double v)                        { return p.set_first_line_indent(v); },    py::arg("first_line_indent"))
        .def("set_start_indent",         [](PNP& p, double v)                        { return p.set_start_indent(v); },         py::arg("start_indent"))
        .def("set_end_indent",           [](PNP& p, double v)                        { return p.set_end_indent(v); },           py::arg("end_indent"))
        .def("set_space_before",         [](PNP& p, double v)                        { return p.set_space_before(v); },         py::arg("space_before"))
        .def("set_space_after",          [](PNP& p, double v)                        { return p.set_space_after(v); },          py::arg("space_after"))
        .def("set_auto_hyphenate",       [](PNP& p, bool v)                          { return p.set_auto_hyphenate(v); },       py::arg("auto_hyphenate"))
        .def("set_hyphenated_word_size", [](PNP& p, int32_t v)                       { return p.set_hyphenated_word_size(v); }, py::arg("hyphenated_word_size"))
        .def("set_pre_hyphen",           [](PNP& p, int32_t v)                       { return p.set_pre_hyphen(v); },           py::arg("pre_hyphen"))
        .def("set_post_hyphen",          [](PNP& p, int32_t v)                       { return p.set_post_hyphen(v); },          py::arg("post_hyphen"))
        .def("set_consecutive_hyphens",  [](PNP& p, int32_t v)                       { return p.set_consecutive_hyphens(v); },  py::arg("consecutive_hyphens"))
        .def("set_zone",                 [](PNP& p, double v)                        { return p.set_zone(v); },                 py::arg("zone"))
        .def("set_word_spacing",         [](PNP& p, const std::vector<double>& v)    { return p.set_word_spacing(v); },         py::arg("word_spacing"))
        .def("set_letter_spacing",       [](PNP& p, const std::vector<double>& v)    { return p.set_letter_spacing(v); },       py::arg("letter_spacing"))
        .def("set_glyph_spacing",        [](PNP& p, const std::vector<double>& v)    { return p.set_glyph_spacing(v); },        py::arg("glyph_spacing"))
        .def("set_auto_leading",         [](PNP& p, double v)                        { return p.set_auto_leading(v); },         py::arg("auto_leading"))
        .def("set_leading_type",         [](PNP& p, TextLayerEnum::LeadingType v)          { return p.set_leading_type(v); },         py::arg("leading_type"))
        .def("set_hanging",              [](PNP& p, bool v)                          { return p.set_hanging(v); },              py::arg("hanging"))
        .def("set_burasagari",           [](PNP& p, bool v)                          { return p.set_burasagari(v); },           py::arg("burasagari"))
        .def("set_kinsoku_order",        [](PNP& p, TextLayerEnum::KinsokuOrder v)         { return p.set_kinsoku_order(v); },        py::arg("kinsoku_order"))
        .def("set_every_line_composer",  [](PNP& p, bool v)                          { return p.set_every_line_composer(v); },  py::arg("every_line_composer"))
        ;

    // -----------------------------------------------------------------------
    //  FontProxy
    // -----------------------------------------------------------------------
    using FP = typename Layer::FontProxy;
    py::class_<FP>(m, ("FontProxy" + extension).c_str())
        .def_property_readonly("postscript_name", [](FP& p){ return p.postscript_name(); })
        .def_property_readonly("name",            [](FP& p){ return p.name(); })
        .def_property_readonly("script",          [](FP& p){ return p.script(); })
        .def_property_readonly("type",            [](FP& p){ return p.type(); })
        .def_property_readonly("synthetic",       [](FP& p){ return p.synthetic(); })
        .def_property_readonly("is_sentinel",     [](FP& p){ return p.is_sentinel(); })
        .def("set_postscript_name", [](FP& p, const std::string& v){ return p.set_postscript_name(v); }, py::arg("name"))
        ;

    // -----------------------------------------------------------------------
    //  FontSetProxy
    // -----------------------------------------------------------------------
    using FSP = typename Layer::FontSetProxy;
    py::class_<FSP>(m, ("FontSetProxy" + extension).c_str())
        .def_property_readonly("count",        [](FSP& p){ return p.count(); })
        .def_property_readonly("used_indices", [](FSP& p){ return p.used_indices(); })
        .def_property_readonly("used_names",   [](FSP& p){ return p.used_names(); })
        .def("find_index", [](FSP& p, const std::string& name){ return p.find_index(name); }, py::arg("postscript_name"))
        .def("add",        [](FSP& p, const std::string& name, TextLayerEnum::FontType font_type, TextLayerEnum::FontScript script, int32_t synthetic){
                               return p.add(name, font_type, script, synthetic);
                           },
             py::arg("postscript_name"),
             py::arg("font_type") = TextLayerEnum::FontType::OpenType,
             py::arg("script") = TextLayerEnum::FontScript::Roman,
             py::arg("synthetic") = 0)
        ;
}
