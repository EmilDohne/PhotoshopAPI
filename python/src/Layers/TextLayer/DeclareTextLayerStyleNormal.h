#pragma once

#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"
#include "LayeredFile/LayerTypes/Layer.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


// ---------------------------------------------------------------------------
//  Style Normal (default style sheet) property bindings
// ---------------------------------------------------------------------------

template <typename T>
void bind_textlayer_style_normal_apis(py::class_<TextLayer<T>, Layer<T>, std::shared_ptr<TextLayer<T>>>& text_layer)
{
    using Class = TextLayer<T>;

    text_layer.def_property_readonly("style_sheet_count", &Class::style_sheet_count, R"pbdoc(

        Number of style sheets found in EngineData /ResourceDict /StyleSheetSet.

    )pbdoc");

    text_layer.def("style_normal_sheet_index", &Class::style_normal_sheet_index, R"pbdoc(

        Read /TheNormalStyleSheet index from EngineData /ResourceDict.

    )pbdoc");

    text_layer.def("set_style_normal_sheet_index", &Class::set_style_normal_sheet_index,
        py::arg("sheet_index"), R"pbdoc(

        Set /TheNormalStyleSheet index in EngineData /ResourceDict.

    )pbdoc");

    text_layer.def("style_normal_font", &Class::style_normal_font, R"pbdoc(

        Read /Font from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_font", &Class::set_style_normal_font,
        py::arg("font"), R"pbdoc(

        Set /Font in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_font_size", &Class::style_normal_font_size, R"pbdoc(

        Read /FontSize from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_font_size", &Class::set_style_normal_font_size,
        py::arg("font_size"), R"pbdoc(

        Set /FontSize in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_leading", &Class::style_normal_leading, R"pbdoc(

        Read /Leading from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_leading", &Class::set_style_normal_leading,
        py::arg("leading"), R"pbdoc(

        Set /Leading in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_auto_leading", &Class::style_normal_auto_leading, R"pbdoc(

        Read /AutoLeading from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_auto_leading", &Class::set_style_normal_auto_leading,
        py::arg("auto_leading"), R"pbdoc(

        Set /AutoLeading in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_kerning", &Class::style_normal_kerning, R"pbdoc(

        Read /Kerning from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_kerning", &Class::set_style_normal_kerning,
        py::arg("kerning"), R"pbdoc(

        Set /Kerning in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_faux_bold", &Class::style_normal_faux_bold, R"pbdoc(

        Read /FauxBold from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_faux_bold", &Class::set_style_normal_faux_bold,
        py::arg("faux_bold"), R"pbdoc(

        Set /FauxBold in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_faux_italic", &Class::style_normal_faux_italic, R"pbdoc(

        Read /FauxItalic from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_faux_italic", &Class::set_style_normal_faux_italic,
        py::arg("faux_italic"), R"pbdoc(

        Set /FauxItalic in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_horizontal_scale", &Class::style_normal_horizontal_scale, R"pbdoc(

        Read /HorizontalScale from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_horizontal_scale", &Class::set_style_normal_horizontal_scale,
        py::arg("horizontal_scale"), R"pbdoc(

        Set /HorizontalScale in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_vertical_scale", &Class::style_normal_vertical_scale, R"pbdoc(

        Read /VerticalScale from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_vertical_scale", &Class::set_style_normal_vertical_scale,
        py::arg("vertical_scale"), R"pbdoc(

        Set /VerticalScale in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_tracking", &Class::style_normal_tracking, R"pbdoc(

        Read /Tracking from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_tracking", &Class::set_style_normal_tracking,
        py::arg("tracking"), R"pbdoc(

        Set /Tracking in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_auto_kerning", &Class::style_normal_auto_kerning, R"pbdoc(

        Read /AutoKerning from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_auto_kerning", &Class::set_style_normal_auto_kerning,
        py::arg("auto_kerning"), R"pbdoc(

        Set /AutoKerning in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_baseline_shift", &Class::style_normal_baseline_shift, R"pbdoc(

        Read /BaselineShift from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_baseline_shift", &Class::set_style_normal_baseline_shift,
        py::arg("baseline_shift"), R"pbdoc(

        Set /BaselineShift in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_font_caps", &Class::style_normal_font_caps, R"pbdoc(

        Read /FontCaps from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_font_caps", &Class::set_style_normal_font_caps,
        py::arg("font_caps"), R"pbdoc(

        Set /FontCaps in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_font_baseline", &Class::style_normal_font_baseline, R"pbdoc(

        Read /FontBaseline from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_font_baseline", &Class::set_style_normal_font_baseline,
        py::arg("font_baseline"), R"pbdoc(

        Set /FontBaseline in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_no_break", &Class::style_normal_no_break, R"pbdoc(

        Read /NoBreak from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_no_break", &Class::set_style_normal_no_break,
        py::arg("no_break"), R"pbdoc(

        Set /NoBreak in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_language", &Class::style_normal_language, R"pbdoc(

        Read /Language from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_language", &Class::set_style_normal_language,
        py::arg("language"), R"pbdoc(

        Set /Language in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_character_direction", &Class::style_normal_character_direction, R"pbdoc(

        Read /CharacterDirection from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_character_direction", &Class::set_style_normal_character_direction,
        py::arg("character_direction"), R"pbdoc(

        Set /CharacterDirection in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_baseline_direction", &Class::style_normal_baseline_direction, R"pbdoc(

        Read /BaselineDirection from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_baseline_direction", &Class::set_style_normal_baseline_direction,
        py::arg("baseline_direction"), R"pbdoc(

        Set /BaselineDirection in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_tsume", &Class::style_normal_tsume, R"pbdoc(

        Read /Tsume from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_tsume", &Class::set_style_normal_tsume,
        py::arg("tsume"), R"pbdoc(

        Set /Tsume in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_kashida", &Class::style_normal_kashida, R"pbdoc(

        Read /Kashida from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_kashida", &Class::set_style_normal_kashida,
        py::arg("kashida"), R"pbdoc(

        Set /Kashida in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_diacritic_pos", &Class::style_normal_diacritic_pos, R"pbdoc(

        Read /DiacriticPos from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_diacritic_pos", &Class::set_style_normal_diacritic_pos,
        py::arg("diacritic_pos"), R"pbdoc(

        Set /DiacriticPos in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_ligatures", &Class::style_normal_ligatures, R"pbdoc(

        Read /Ligatures from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_ligatures", &Class::set_style_normal_ligatures,
        py::arg("ligatures"), R"pbdoc(

        Set /Ligatures in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_dligatures", &Class::style_normal_dligatures, R"pbdoc(

        Read /DLigatures from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_dligatures", &Class::set_style_normal_dligatures,
        py::arg("dligatures"), R"pbdoc(

        Set /DLigatures in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_underline", &Class::style_normal_underline, R"pbdoc(

        Read /Underline from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_underline", &Class::set_style_normal_underline,
        py::arg("underline"), R"pbdoc(

        Set /Underline in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_strikethrough", &Class::style_normal_strikethrough, R"pbdoc(

        Read /Strikethrough from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_strikethrough", &Class::set_style_normal_strikethrough,
        py::arg("strikethrough"), R"pbdoc(

        Set /Strikethrough in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_stroke_flag", &Class::style_normal_stroke_flag, R"pbdoc(

        Read /StrokeFlag from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_stroke_flag", &Class::set_style_normal_stroke_flag,
        py::arg("stroke_flag"), R"pbdoc(

        Set /StrokeFlag in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_fill_flag", &Class::style_normal_fill_flag, R"pbdoc(

        Read /FillFlag from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_fill_flag", &Class::set_style_normal_fill_flag,
        py::arg("fill_flag"), R"pbdoc(

        Set /FillFlag in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_fill_first", &Class::style_normal_fill_first, R"pbdoc(

        Read /FillFirst from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_fill_first", &Class::set_style_normal_fill_first,
        py::arg("fill_first"), R"pbdoc(

        Set /FillFirst in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_outline_width", &Class::style_normal_outline_width, R"pbdoc(

        Read /OutlineWidth from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_outline_width", &Class::set_style_normal_outline_width,
        py::arg("outline_width"), R"pbdoc(

        Set /OutlineWidth in the normal style sheet.

    )pbdoc");

    text_layer.def("style_normal_fill_color", &Class::style_normal_fill_color, R"pbdoc(

        Read /FillColor /Values from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_fill_color", &Class::set_style_normal_fill_color,
        py::arg("values"), R"pbdoc(

        Set /FillColor /Values in the normal style sheet. Expects 4 values.

    )pbdoc");

    text_layer.def("style_normal_stroke_color", &Class::style_normal_stroke_color, R"pbdoc(

        Read /StrokeColor /Values from the normal style sheet.

    )pbdoc");

    text_layer.def("set_style_normal_stroke_color", &Class::set_style_normal_stroke_color,
        py::arg("values"), R"pbdoc(

        Set /StrokeColor /Values in the normal style sheet. Expects 4 values.

    )pbdoc");
}
