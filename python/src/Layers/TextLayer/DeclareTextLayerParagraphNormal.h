#pragma once

#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"
#include "LayeredFile/LayerTypes/Layer.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


// ---------------------------------------------------------------------------
//  Paragraph Normal (default paragraph sheet) property bindings
// ---------------------------------------------------------------------------

template <typename T>
void bind_textlayer_paragraph_normal_apis(py::class_<TextLayer<T>, Layer<T>, std::shared_ptr<TextLayer<T>>>& text_layer)
{
    using Class = TextLayer<T>;

    text_layer.def_property_readonly("paragraph_sheet_count", &Class::paragraph_sheet_count, R"pbdoc(

        Number of paragraph sheets found in EngineData /ResourceDict /ParagraphSheetSet.

    )pbdoc");

    text_layer.def("paragraph_normal_sheet_index", &Class::paragraph_normal_sheet_index, R"pbdoc(

        Read /TheNormalParagraphSheet index from EngineData /ResourceDict.

    )pbdoc");

    text_layer.def("set_paragraph_normal_sheet_index", &Class::set_paragraph_normal_sheet_index,
        py::arg("sheet_index"), R"pbdoc(

        Set /TheNormalParagraphSheet index in EngineData /ResourceDict.

    )pbdoc");

    text_layer.def("paragraph_normal_justification", &Class::paragraph_normal_justification, R"pbdoc(

        Read /Justification from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_justification", &Class::set_paragraph_normal_justification,
        py::arg("justification"), R"pbdoc(

        Set /Justification in the normal paragraph sheet.

    )pbdoc");

    text_layer.def("paragraph_normal_first_line_indent", &Class::paragraph_normal_first_line_indent, R"pbdoc(

        Read /FirstLineIndent from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_first_line_indent", &Class::set_paragraph_normal_first_line_indent,
        py::arg("first_line_indent"), R"pbdoc(

        Set /FirstLineIndent in the normal paragraph sheet.

    )pbdoc");

    text_layer.def("paragraph_normal_start_indent", &Class::paragraph_normal_start_indent, R"pbdoc(

        Read /StartIndent from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_start_indent", &Class::set_paragraph_normal_start_indent,
        py::arg("start_indent"), R"pbdoc(

        Set /StartIndent in the normal paragraph sheet.

    )pbdoc");

    text_layer.def("paragraph_normal_end_indent", &Class::paragraph_normal_end_indent, R"pbdoc(

        Read /EndIndent from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_end_indent", &Class::set_paragraph_normal_end_indent,
        py::arg("end_indent"), R"pbdoc(

        Set /EndIndent in the normal paragraph sheet.

    )pbdoc");

    text_layer.def("paragraph_normal_space_before", &Class::paragraph_normal_space_before, R"pbdoc(

        Read /SpaceBefore from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_space_before", &Class::set_paragraph_normal_space_before,
        py::arg("space_before"), R"pbdoc(

        Set /SpaceBefore in the normal paragraph sheet.

    )pbdoc");

    text_layer.def("paragraph_normal_space_after", &Class::paragraph_normal_space_after, R"pbdoc(

        Read /SpaceAfter from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_space_after", &Class::set_paragraph_normal_space_after,
        py::arg("space_after"), R"pbdoc(

        Set /SpaceAfter in the normal paragraph sheet.

    )pbdoc");

    text_layer.def("paragraph_normal_auto_hyphenate", &Class::paragraph_normal_auto_hyphenate, R"pbdoc(

        Read /AutoHyphenate from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_auto_hyphenate", &Class::set_paragraph_normal_auto_hyphenate,
        py::arg("auto_hyphenate"), R"pbdoc(

        Set /AutoHyphenate in the normal paragraph sheet.

    )pbdoc");

    text_layer.def("paragraph_normal_hyphenated_word_size", &Class::paragraph_normal_hyphenated_word_size, R"pbdoc(

        Read /HyphenatedWordSize from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_hyphenated_word_size", &Class::set_paragraph_normal_hyphenated_word_size,
        py::arg("hyphenated_word_size"), R"pbdoc(

        Set /HyphenatedWordSize in the normal paragraph sheet.

    )pbdoc");

    text_layer.def("paragraph_normal_pre_hyphen", &Class::paragraph_normal_pre_hyphen, R"pbdoc(

        Read /PreHyphen from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_pre_hyphen", &Class::set_paragraph_normal_pre_hyphen,
        py::arg("pre_hyphen"), R"pbdoc(

        Set /PreHyphen in the normal paragraph sheet.

    )pbdoc");

    text_layer.def("paragraph_normal_post_hyphen", &Class::paragraph_normal_post_hyphen, R"pbdoc(

        Read /PostHyphen from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_post_hyphen", &Class::set_paragraph_normal_post_hyphen,
        py::arg("post_hyphen"), R"pbdoc(

        Set /PostHyphen in the normal paragraph sheet.

    )pbdoc");

    text_layer.def("paragraph_normal_consecutive_hyphens", &Class::paragraph_normal_consecutive_hyphens, R"pbdoc(

        Read /ConsecutiveHyphens from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_consecutive_hyphens", &Class::set_paragraph_normal_consecutive_hyphens,
        py::arg("consecutive_hyphens"), R"pbdoc(

        Set /ConsecutiveHyphens in the normal paragraph sheet.

    )pbdoc");

    text_layer.def("paragraph_normal_zone", &Class::paragraph_normal_zone, R"pbdoc(

        Read /Zone from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_zone", &Class::set_paragraph_normal_zone,
        py::arg("zone"), R"pbdoc(

        Set /Zone in the normal paragraph sheet.

    )pbdoc");

    text_layer.def("paragraph_normal_word_spacing", &Class::paragraph_normal_word_spacing, R"pbdoc(

        Read /WordSpacing from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_word_spacing", &Class::set_paragraph_normal_word_spacing,
        py::arg("word_spacing"), R"pbdoc(

        Set /WordSpacing in the normal paragraph sheet.

    )pbdoc");

    text_layer.def("paragraph_normal_letter_spacing", &Class::paragraph_normal_letter_spacing, R"pbdoc(

        Read /LetterSpacing from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_letter_spacing", &Class::set_paragraph_normal_letter_spacing,
        py::arg("letter_spacing"), R"pbdoc(

        Set /LetterSpacing in the normal paragraph sheet.

    )pbdoc");

    text_layer.def("paragraph_normal_glyph_spacing", &Class::paragraph_normal_glyph_spacing, R"pbdoc(

        Read /GlyphSpacing from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_glyph_spacing", &Class::set_paragraph_normal_glyph_spacing,
        py::arg("glyph_spacing"), R"pbdoc(

        Set /GlyphSpacing in the normal paragraph sheet.

    )pbdoc");

    text_layer.def("paragraph_normal_auto_leading", &Class::paragraph_normal_auto_leading, R"pbdoc(

        Read /AutoLeading from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_auto_leading", &Class::set_paragraph_normal_auto_leading,
        py::arg("auto_leading"), R"pbdoc(

        Set /AutoLeading in the normal paragraph sheet.

    )pbdoc");

    text_layer.def("paragraph_normal_leading_type", &Class::paragraph_normal_leading_type, R"pbdoc(

        Read /LeadingType from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_leading_type", &Class::set_paragraph_normal_leading_type,
        py::arg("leading_type"), R"pbdoc(

        Set /LeadingType in the normal paragraph sheet.

    )pbdoc");

    text_layer.def("paragraph_normal_hanging", &Class::paragraph_normal_hanging, R"pbdoc(

        Read /Hanging from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_hanging", &Class::set_paragraph_normal_hanging,
        py::arg("hanging"), R"pbdoc(

        Set /Hanging in the normal paragraph sheet.

    )pbdoc");

    text_layer.def("paragraph_normal_burasagari", &Class::paragraph_normal_burasagari, R"pbdoc(

        Read /Burasagari from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_burasagari", &Class::set_paragraph_normal_burasagari,
        py::arg("burasagari"), R"pbdoc(

        Set /Burasagari in the normal paragraph sheet.

    )pbdoc");

    text_layer.def("paragraph_normal_kinsoku_order", &Class::paragraph_normal_kinsoku_order, R"pbdoc(

        Read /KinsokuOrder from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_kinsoku_order", &Class::set_paragraph_normal_kinsoku_order,
        py::arg("kinsoku_order"), R"pbdoc(

        Set /KinsokuOrder in the normal paragraph sheet.

    )pbdoc");

    text_layer.def("paragraph_normal_every_line_composer", &Class::paragraph_normal_every_line_composer, R"pbdoc(

        Read /EveryLineComposer from the normal paragraph sheet.

    )pbdoc");

    text_layer.def("set_paragraph_normal_every_line_composer", &Class::set_paragraph_normal_every_line_composer,
        py::arg("every_line_composer"), R"pbdoc(

        Set /EveryLineComposer in the normal paragraph sheet.

    )pbdoc");
}
