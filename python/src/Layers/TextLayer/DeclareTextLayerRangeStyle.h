#pragma once

// ---------------------------------------------------------------------------
//  DeclareTextLayerRangeStyle.h  —  pybind11 bindings for the range-based
//  styling API (CharacterStyleRange, ParagraphStyleRange).
// ---------------------------------------------------------------------------

#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"
#include "LayeredFile/LayerTypes/TextLayer/TextLayerEnum.h"
#include "Macros.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <string>
#include <vector>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


// ---------------------------------------------------------------------------
//  declare_character_style_range<T>  —  register CharacterStyleRange as a
//  Python class and bind all chainable setters.
// ---------------------------------------------------------------------------
template <typename T>
void declare_character_style_range(py::module& m, const std::string& extension)
{
    using Layer = TextLayer<T>;
    using Mixin = TextLayerRangeStyleMixin<Layer>;
    using CSR   = typename Mixin::CharacterStyleRange;

    std::string name = "CharacterStyleRange" + extension;
    py::class_<CSR>(m, name.c_str(), R"pbdoc(

        A proxy object for applying character styles to a range of text.

        Obtained via ``style_range()``, ``style_text()``, or ``style_all()`` on a
        TextLayer.  All setter methods are chainable and return the same proxy::

            layer.style_text("Bold").set_bold(True).set_fill_color([1, 0, 0, 0])

        If the range doesn't match any text (e.g. ``style_text("xyz")`` when "xyz"
        doesn't appear), all setter calls are safe no-ops.

    )pbdoc")
        .def("valid", &CSR::valid, "True if this proxy targets at least one character range.")
        .def("range_count", &CSR::range_count, "Number of character ranges this proxy covers.")
        // Character style setters
        .def("set_font_size",       &CSR::set_font_size,       py::arg("size"),  "Set font size in points.", py::return_value_policy::reference_internal)
        .def("set_leading",         &CSR::set_leading,         py::arg("value"), "Set leading (line spacing).", py::return_value_policy::reference_internal)
        .def("set_auto_leading",    &CSR::set_auto_leading,    py::arg("value"), "Enable/disable auto leading.", py::return_value_policy::reference_internal)
        .def("set_kerning",         &CSR::set_kerning,         py::arg("value"), "Set kerning value.", py::return_value_policy::reference_internal)
        .def("set_bold",            &CSR::set_bold,            py::arg("value"), "Enable/disable faux bold.", py::return_value_policy::reference_internal)
        .def("set_italic",          &CSR::set_italic,          py::arg("value"), "Enable/disable faux italic.", py::return_value_policy::reference_internal)
        .def("set_underline",       &CSR::set_underline,       py::arg("value"), "Enable/disable underline.", py::return_value_policy::reference_internal)
        .def("set_strikethrough",   &CSR::set_strikethrough,   py::arg("value"), "Enable/disable strikethrough.", py::return_value_policy::reference_internal)
        .def("set_fill_color",      &CSR::set_fill_color,      py::arg("color"), "Set fill color [A,R,G,B] floats.", py::return_value_policy::reference_internal)
        .def("set_stroke_color",    &CSR::set_stroke_color,    py::arg("color"), "Set stroke color [A,R,G,B] floats.", py::return_value_policy::reference_internal)
        .def("set_stroke_flag",     &CSR::set_stroke_flag,     py::arg("value"), "Enable/disable stroke.", py::return_value_policy::reference_internal)
        .def("set_fill_flag",       &CSR::set_fill_flag,       py::arg("value"), "Enable/disable fill.", py::return_value_policy::reference_internal)
        .def("set_fill_first",      &CSR::set_fill_first,      py::arg("value"), "Set fill-first draw order.", py::return_value_policy::reference_internal)
        .def("set_outline_width",   &CSR::set_outline_width,   py::arg("width"), "Set outline/stroke width.", py::return_value_policy::reference_internal)
        .def("set_horizontal_scale",&CSR::set_horizontal_scale,py::arg("value"), "Set horizontal scale.", py::return_value_policy::reference_internal)
        .def("set_vertical_scale",  &CSR::set_vertical_scale,  py::arg("value"), "Set vertical scale.", py::return_value_policy::reference_internal)
        .def("set_tracking",        &CSR::set_tracking,        py::arg("value"), "Set tracking value.", py::return_value_policy::reference_internal)
        .def("set_auto_kerning",    &CSR::set_auto_kerning,    py::arg("value"), "Enable/disable auto kerning.", py::return_value_policy::reference_internal)
        .def("set_baseline_shift",  &CSR::set_baseline_shift,  py::arg("value"), "Set baseline shift.", py::return_value_policy::reference_internal)
        .def("set_font_caps",       &CSR::set_font_caps,       py::arg("value"), "Set font caps style.", py::return_value_policy::reference_internal)
        .def("set_font_baseline",   &CSR::set_font_baseline,   py::arg("value"), "Set font baseline style.", py::return_value_policy::reference_internal)
        .def("set_no_break",        &CSR::set_no_break,        py::arg("value"), "Enable/disable no-break.", py::return_value_policy::reference_internal)
        .def("set_language",        &CSR::set_language,        py::arg("value"), "Set language code.", py::return_value_policy::reference_internal)
        .def("set_ligatures",       &CSR::set_ligatures,       py::arg("value"), "Enable/disable standard ligatures.", py::return_value_policy::reference_internal)
        .def("set_dligatures",      &CSR::set_dligatures,      py::arg("value"), "Enable/disable discretionary ligatures.", py::return_value_policy::reference_internal)
        // Font management
        .def("set_font", &CSR::set_font, py::arg("postscript_name"),
            "Set font by PostScript name. Adds to FontSet if not present.", py::return_value_policy::reference_internal)
        .def("set_font_index", &CSR::set_font_index, py::arg("index"),
            "Set font by index into the FontSet.", py::return_value_policy::reference_internal)
    ;
}


// ---------------------------------------------------------------------------
//  declare_paragraph_style_range<T>
// ---------------------------------------------------------------------------
template <typename T>
void declare_paragraph_style_range(py::module& m, const std::string& extension)
{
    using Layer = TextLayer<T>;
    using Mixin = TextLayerRangeStyleMixin<Layer>;
    using PSR   = typename Mixin::ParagraphStyleRange;

    std::string name = "ParagraphStyleRange" + extension;
    py::class_<PSR>(m, name.c_str(), R"pbdoc(

        A proxy object for applying paragraph styles to a range of text.

        Obtained via ``paragraph_range()``, ``paragraph_text()``, or
        ``paragraph_all()`` on a TextLayer.  All setters are chainable.

    )pbdoc")
        .def("valid", &PSR::valid, "True if this proxy targets at least one range.")
        .def("range_count", &PSR::range_count, "Number of ranges this proxy covers.")
        .def("set_justification",       &PSR::set_justification,       py::arg("value"), py::return_value_policy::reference_internal)
        .def("set_first_line_indent",   &PSR::set_first_line_indent,   py::arg("value"), py::return_value_policy::reference_internal)
        .def("set_start_indent",        &PSR::set_start_indent,        py::arg("value"), py::return_value_policy::reference_internal)
        .def("set_end_indent",          &PSR::set_end_indent,          py::arg("value"), py::return_value_policy::reference_internal)
        .def("set_space_before",        &PSR::set_space_before,        py::arg("value"), py::return_value_policy::reference_internal)
        .def("set_space_after",         &PSR::set_space_after,         py::arg("value"), py::return_value_policy::reference_internal)
        .def("set_auto_hyphenate",      &PSR::set_auto_hyphenate,      py::arg("value"), py::return_value_policy::reference_internal)
        .def("set_hyphenated_word_size",&PSR::set_hyphenated_word_size,py::arg("value"), py::return_value_policy::reference_internal)
        .def("set_pre_hyphen",          &PSR::set_pre_hyphen,          py::arg("value"), py::return_value_policy::reference_internal)
        .def("set_post_hyphen",         &PSR::set_post_hyphen,         py::arg("value"), py::return_value_policy::reference_internal)
        .def("set_consecutive_hyphens", &PSR::set_consecutive_hyphens, py::arg("value"), py::return_value_policy::reference_internal)
        .def("set_zone",                &PSR::set_zone,                py::arg("value"), py::return_value_policy::reference_internal)
        .def("set_word_spacing",        &PSR::set_word_spacing,        py::arg("value"), py::return_value_policy::reference_internal)
        .def("set_letter_spacing",      &PSR::set_letter_spacing,      py::arg("value"), py::return_value_policy::reference_internal)
        .def("set_glyph_spacing",       &PSR::set_glyph_spacing,       py::arg("value"), py::return_value_policy::reference_internal)
        .def("set_auto_leading",        &PSR::set_auto_leading,        py::arg("value"), py::return_value_policy::reference_internal)
        .def("set_leading_type",        &PSR::set_leading_type,        py::arg("value"), py::return_value_policy::reference_internal)
        .def("set_hanging",             &PSR::set_hanging,             py::arg("value"), py::return_value_policy::reference_internal)
        .def("set_burasagari",          &PSR::set_burasagari,          py::arg("value"), py::return_value_policy::reference_internal)
        .def("set_kinsoku_order",       &PSR::set_kinsoku_order,       py::arg("value"), py::return_value_policy::reference_internal)
        .def("set_every_line_composer", &PSR::set_every_line_composer, py::arg("value"), py::return_value_policy::reference_internal)
    ;
}


// ---------------------------------------------------------------------------
//  bind_textlayer_range_style_apis<T>  —  add style_range/style_text/style_all
//  and paragraph_range/paragraph_text/paragraph_all to the TextLayer PyClass.
// ---------------------------------------------------------------------------
template <typename T>
void bind_textlayer_range_style_apis(
    py::class_<TextLayer<T>, Layer<T>, std::shared_ptr<TextLayer<T>>>& text_layer)
{
    using Class = TextLayer<T>;

    // --- Character style range ---
    text_layer.def("style_range", &Class::style_range,
        py::arg("start"), py::arg("end"),
        R"pbdoc(

        Get a CharacterStyleRange proxy for characters [start, end).

        Positions are in UTF-16 code units (same as style_run_lengths()).
        The proxy handles run splitting automatically.

        :param start: Start position (inclusive)
        :param end: End position (exclusive)
        :return: CharacterStyleRange proxy with chainable setters

    )pbdoc");

    text_layer.def("style_text", &Class::style_text,
        py::arg("needle"), py::arg("occurrence") = 0,
        R"pbdoc(

        Get a CharacterStyleRange proxy targeting occurrences of a substring.

        :param needle: Text substring to find (UTF-8)
        :param occurrence: 0 = all occurrences (default), 1 = first, 2 = second, ...
        :return: CharacterStyleRange proxy (empty/no-op if not found)

    )pbdoc");

    text_layer.def("style_all", &Class::style_all,
        R"pbdoc(

        Get a CharacterStyleRange proxy targeting the entire text content.

        :return: CharacterStyleRange proxy covering all characters

    )pbdoc");

    // --- Paragraph style range ---
    text_layer.def("paragraph_range", &Class::paragraph_range,
        py::arg("start"), py::arg("end"),
        R"pbdoc(

        Get a ParagraphStyleRange proxy for characters [start, end).

        :param start: Start position (inclusive, UTF-16 code units)
        :param end: End position (exclusive, UTF-16 code units)
        :return: ParagraphStyleRange proxy with chainable setters

    )pbdoc");

    text_layer.def("paragraph_text", &Class::paragraph_text,
        py::arg("needle"), py::arg("occurrence") = 0,
        R"pbdoc(

        Get a ParagraphStyleRange proxy targeting occurrences of a substring.

        :param needle: Text substring to find (UTF-8)
        :param occurrence: 0 = all occurrences (default), 1 = first, 2 = second, ...
        :return: ParagraphStyleRange proxy

    )pbdoc");

    text_layer.def("paragraph_all", &Class::paragraph_all,
        R"pbdoc(

        Get a ParagraphStyleRange proxy targeting the entire text content.

        :return: ParagraphStyleRange proxy covering all paragraphs

    )pbdoc");

    // --- Also expose paragraph_run_lengths and split_paragraph_run ---
    text_layer.def("paragraph_run_lengths", &Class::paragraph_run_lengths,
        "Get paragraph run lengths (UTF-16 code units), or None.");

    text_layer.def("split_paragraph_run", &Class::split_paragraph_run,
        py::arg("run_index"), py::arg("char_offset"),
        "Split a paragraph run at the given character offset.");
}
