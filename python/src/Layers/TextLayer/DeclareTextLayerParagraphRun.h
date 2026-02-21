#pragma once

#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"
#include "LayeredFile/LayerTypes/Layer.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


// ---------------------------------------------------------------------------
//  Paragraph Run property bindings (read + write for each paragraph-run key)
// ---------------------------------------------------------------------------

template <typename T>
void bind_textlayer_paragraph_run_apis(py::class_<TextLayer<T>, Layer<T>, std::shared_ptr<TextLayer<T>>>& text_layer)
{
    using Class = TextLayer<T>;

    text_layer.def("paragraph_run_justification", &Class::paragraph_run_justification, py::arg("run_index"), R"pbdoc(

        Read /Justification for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_justification", &Class::set_paragraph_run_justification,
        py::arg("run_index"),
        py::arg("justification"), R"pbdoc(

        Set /Justification for a paragraph run.

    )pbdoc");

    text_layer.def("paragraph_run_first_line_indent", &Class::paragraph_run_first_line_indent, py::arg("run_index"), R"pbdoc(

        Read /FirstLineIndent for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_first_line_indent", &Class::set_paragraph_run_first_line_indent,
        py::arg("run_index"),
        py::arg("first_line_indent"), R"pbdoc(

        Set /FirstLineIndent for a paragraph run.

    )pbdoc");

    text_layer.def("paragraph_run_start_indent", &Class::paragraph_run_start_indent, py::arg("run_index"), R"pbdoc(

        Read /StartIndent for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_start_indent", &Class::set_paragraph_run_start_indent,
        py::arg("run_index"),
        py::arg("start_indent"), R"pbdoc(

        Set /StartIndent for a paragraph run.

    )pbdoc");

    text_layer.def("paragraph_run_end_indent", &Class::paragraph_run_end_indent, py::arg("run_index"), R"pbdoc(

        Read /EndIndent for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_end_indent", &Class::set_paragraph_run_end_indent,
        py::arg("run_index"),
        py::arg("end_indent"), R"pbdoc(

        Set /EndIndent for a paragraph run.

    )pbdoc");

    text_layer.def("paragraph_run_space_before", &Class::paragraph_run_space_before, py::arg("run_index"), R"pbdoc(

        Read /SpaceBefore for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_space_before", &Class::set_paragraph_run_space_before,
        py::arg("run_index"),
        py::arg("space_before"), R"pbdoc(

        Set /SpaceBefore for a paragraph run.

    )pbdoc");

    text_layer.def("paragraph_run_space_after", &Class::paragraph_run_space_after, py::arg("run_index"), R"pbdoc(

        Read /SpaceAfter for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_space_after", &Class::set_paragraph_run_space_after,
        py::arg("run_index"),
        py::arg("space_after"), R"pbdoc(

        Set /SpaceAfter for a paragraph run.

    )pbdoc");

    text_layer.def("paragraph_run_auto_hyphenate", &Class::paragraph_run_auto_hyphenate, py::arg("run_index"), R"pbdoc(

        Read /AutoHyphenate for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_auto_hyphenate", &Class::set_paragraph_run_auto_hyphenate,
        py::arg("run_index"),
        py::arg("auto_hyphenate"), R"pbdoc(

        Set /AutoHyphenate for a paragraph run.

    )pbdoc");

    text_layer.def("paragraph_run_hyphenated_word_size", &Class::paragraph_run_hyphenated_word_size, py::arg("run_index"), R"pbdoc(

        Read /HyphenatedWordSize for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_hyphenated_word_size", &Class::set_paragraph_run_hyphenated_word_size,
        py::arg("run_index"),
        py::arg("hyphenated_word_size"), R"pbdoc(

        Set /HyphenatedWordSize for a paragraph run.

    )pbdoc");

    text_layer.def("paragraph_run_pre_hyphen", &Class::paragraph_run_pre_hyphen, py::arg("run_index"), R"pbdoc(

        Read /PreHyphen for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_pre_hyphen", &Class::set_paragraph_run_pre_hyphen,
        py::arg("run_index"),
        py::arg("pre_hyphen"), R"pbdoc(

        Set /PreHyphen for a paragraph run.

    )pbdoc");

    text_layer.def("paragraph_run_post_hyphen", &Class::paragraph_run_post_hyphen, py::arg("run_index"), R"pbdoc(

        Read /PostHyphen for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_post_hyphen", &Class::set_paragraph_run_post_hyphen,
        py::arg("run_index"),
        py::arg("post_hyphen"), R"pbdoc(

        Set /PostHyphen for a paragraph run.

    )pbdoc");

    text_layer.def("paragraph_run_consecutive_hyphens", &Class::paragraph_run_consecutive_hyphens, py::arg("run_index"), R"pbdoc(

        Read /ConsecutiveHyphens for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_consecutive_hyphens", &Class::set_paragraph_run_consecutive_hyphens,
        py::arg("run_index"),
        py::arg("consecutive_hyphens"), R"pbdoc(

        Set /ConsecutiveHyphens for a paragraph run.

    )pbdoc");

    text_layer.def("paragraph_run_zone", &Class::paragraph_run_zone, py::arg("run_index"), R"pbdoc(

        Read /Zone for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_zone", &Class::set_paragraph_run_zone,
        py::arg("run_index"),
        py::arg("zone"), R"pbdoc(

        Set /Zone for a paragraph run.

    )pbdoc");

    text_layer.def("paragraph_run_word_spacing", &Class::paragraph_run_word_spacing, py::arg("run_index"), R"pbdoc(

        Read /WordSpacing for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_word_spacing", &Class::set_paragraph_run_word_spacing,
        py::arg("run_index"),
        py::arg("word_spacing"), R"pbdoc(

        Set /WordSpacing for a paragraph run.

    )pbdoc");

    text_layer.def("paragraph_run_letter_spacing", &Class::paragraph_run_letter_spacing, py::arg("run_index"), R"pbdoc(

        Read /LetterSpacing for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_letter_spacing", &Class::set_paragraph_run_letter_spacing,
        py::arg("run_index"),
        py::arg("letter_spacing"), R"pbdoc(

        Set /LetterSpacing for a paragraph run.

    )pbdoc");

    text_layer.def("paragraph_run_glyph_spacing", &Class::paragraph_run_glyph_spacing, py::arg("run_index"), R"pbdoc(

        Read /GlyphSpacing for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_glyph_spacing", &Class::set_paragraph_run_glyph_spacing,
        py::arg("run_index"),
        py::arg("glyph_spacing"), R"pbdoc(

        Set /GlyphSpacing for a paragraph run.

    )pbdoc");

    text_layer.def("paragraph_run_auto_leading", &Class::paragraph_run_auto_leading, py::arg("run_index"), R"pbdoc(

        Read /AutoLeading for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_auto_leading", &Class::set_paragraph_run_auto_leading,
        py::arg("run_index"),
        py::arg("auto_leading"), R"pbdoc(

        Set /AutoLeading for a paragraph run.

    )pbdoc");

    text_layer.def("paragraph_run_leading_type", &Class::paragraph_run_leading_type, py::arg("run_index"), R"pbdoc(

        Read /LeadingType for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_leading_type", &Class::set_paragraph_run_leading_type,
        py::arg("run_index"),
        py::arg("leading_type"), R"pbdoc(

        Set /LeadingType for a paragraph run.

    )pbdoc");

    text_layer.def("paragraph_run_hanging", &Class::paragraph_run_hanging, py::arg("run_index"), R"pbdoc(

        Read /Hanging for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_hanging", &Class::set_paragraph_run_hanging,
        py::arg("run_index"),
        py::arg("hanging"), R"pbdoc(

        Set /Hanging for a paragraph run.

    )pbdoc");

    text_layer.def("paragraph_run_burasagari", &Class::paragraph_run_burasagari, py::arg("run_index"), R"pbdoc(

        Read /Burasagari for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_burasagari", &Class::set_paragraph_run_burasagari,
        py::arg("run_index"),
        py::arg("burasagari"), R"pbdoc(

        Set /Burasagari for a paragraph run.

    )pbdoc");

    text_layer.def("paragraph_run_kinsoku_order", &Class::paragraph_run_kinsoku_order, py::arg("run_index"), R"pbdoc(

        Read /KinsokuOrder for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_kinsoku_order", &Class::set_paragraph_run_kinsoku_order,
        py::arg("run_index"),
        py::arg("kinsoku_order"), R"pbdoc(

        Set /KinsokuOrder for a paragraph run.

    )pbdoc");

    text_layer.def("paragraph_run_every_line_composer", &Class::paragraph_run_every_line_composer, py::arg("run_index"), R"pbdoc(

        Read /EveryLineComposer for a paragraph run.

    )pbdoc");

    text_layer.def("set_paragraph_run_every_line_composer", &Class::set_paragraph_run_every_line_composer,
        py::arg("run_index"),
        py::arg("every_line_composer"), R"pbdoc(

        Set /EveryLineComposer for a paragraph run.

    )pbdoc");
}
