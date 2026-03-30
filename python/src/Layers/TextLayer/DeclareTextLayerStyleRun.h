#pragma once

#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"
#include "LayeredFile/LayerTypes/Layer.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


// ---------------------------------------------------------------------------
//  Style Run property bindings (read + write for each EngineData style-run key)
// ---------------------------------------------------------------------------

template <typename T>
void bind_textlayer_style_run_apis(py::class_<TextLayer<T>, Layer<T>, std::shared_ptr<TextLayer<T>>>& text_layer)
{
    using Class = TextLayer<T>;

    text_layer.def("style_run_font_size", &Class::style_run_font_size, py::arg("run_index"), R"pbdoc(

        Read /FontSize for a style run.

    )pbdoc");

    text_layer.def("set_style_run_font_size", &Class::set_style_run_font_size,
        py::arg("run_index"),
        py::arg("font_size"), R"pbdoc(

        Set /FontSize for a style run.

    )pbdoc");

    text_layer.def("style_run_leading", &Class::style_run_leading, py::arg("run_index"), R"pbdoc(

        Read /Leading for a style run.

    )pbdoc");

    text_layer.def("set_style_run_leading", &Class::set_style_run_leading,
        py::arg("run_index"),
        py::arg("leading"), R"pbdoc(

        Set /Leading for a style run.

    )pbdoc");

    text_layer.def("style_run_auto_leading", &Class::style_run_auto_leading, py::arg("run_index"), R"pbdoc(

        Read /AutoLeading for a style run.

    )pbdoc");

    text_layer.def("set_style_run_auto_leading", &Class::set_style_run_auto_leading,
        py::arg("run_index"),
        py::arg("auto_leading"), R"pbdoc(

        Set /AutoLeading for a style run.

    )pbdoc");

    text_layer.def("style_run_kerning", &Class::style_run_kerning, py::arg("run_index"), R"pbdoc(

        Read /Kerning for a style run.

    )pbdoc");

    text_layer.def("set_style_run_kerning", &Class::set_style_run_kerning,
        py::arg("run_index"),
        py::arg("kerning"), R"pbdoc(

        Set /Kerning for a style run.

    )pbdoc");

    text_layer.def("style_run_fill_color", &Class::style_run_fill_color, py::arg("run_index"), R"pbdoc(

        Read /FillColor /Values array for a style run.

    )pbdoc");

    text_layer.def("set_style_run_fill_color", &Class::set_style_run_fill_color,
        py::arg("run_index"),
        py::arg("values"), R"pbdoc(

        Set /FillColor /Values array for a style run. Expects 4 values.

    )pbdoc");

    text_layer.def("style_run_stroke_color", &Class::style_run_stroke_color, py::arg("run_index"), R"pbdoc(

        Read /StrokeColor /Values array for a style run.

    )pbdoc");

    text_layer.def("set_style_run_stroke_color", &Class::set_style_run_stroke_color,
        py::arg("run_index"),
        py::arg("values"), R"pbdoc(

        Set /StrokeColor /Values array for a style run. Expects 4 values.

    )pbdoc");

    text_layer.def("style_run_font", &Class::style_run_font, py::arg("run_index"), R"pbdoc(

        Read /Font for a style run.

    )pbdoc");

    text_layer.def("set_style_run_font", &Class::set_style_run_font,
        py::arg("run_index"),
        py::arg("font"), R"pbdoc(

        Set /Font for a style run.

    )pbdoc");

    text_layer.def("style_run_faux_bold", &Class::style_run_faux_bold, py::arg("run_index"), R"pbdoc(

        Read /FauxBold for a style run.

    )pbdoc");

    text_layer.def("set_style_run_faux_bold", &Class::set_style_run_faux_bold,
        py::arg("run_index"),
        py::arg("faux_bold"), R"pbdoc(

        Set /FauxBold for a style run.

    )pbdoc");

    text_layer.def("style_run_faux_italic", &Class::style_run_faux_italic, py::arg("run_index"), R"pbdoc(

        Read /FauxItalic for a style run.

    )pbdoc");

    text_layer.def("set_style_run_faux_italic", &Class::set_style_run_faux_italic,
        py::arg("run_index"),
        py::arg("faux_italic"), R"pbdoc(

        Set /FauxItalic for a style run.

    )pbdoc");

    text_layer.def("style_run_horizontal_scale", &Class::style_run_horizontal_scale, py::arg("run_index"), R"pbdoc(

        Read /HorizontalScale for a style run.

    )pbdoc");

    text_layer.def("set_style_run_horizontal_scale", &Class::set_style_run_horizontal_scale,
        py::arg("run_index"),
        py::arg("horizontal_scale"), R"pbdoc(

        Set /HorizontalScale for a style run.

    )pbdoc");

    text_layer.def("style_run_vertical_scale", &Class::style_run_vertical_scale, py::arg("run_index"), R"pbdoc(

        Read /VerticalScale for a style run.

    )pbdoc");

    text_layer.def("set_style_run_vertical_scale", &Class::set_style_run_vertical_scale,
        py::arg("run_index"),
        py::arg("vertical_scale"), R"pbdoc(

        Set /VerticalScale for a style run.

    )pbdoc");

    text_layer.def("style_run_tracking", &Class::style_run_tracking, py::arg("run_index"), R"pbdoc(

        Read /Tracking for a style run.

    )pbdoc");

    text_layer.def("set_style_run_tracking", &Class::set_style_run_tracking,
        py::arg("run_index"),
        py::arg("tracking"), R"pbdoc(

        Set /Tracking for a style run.

    )pbdoc");

    text_layer.def("style_run_auto_kerning", &Class::style_run_auto_kerning, py::arg("run_index"), R"pbdoc(

        Read /AutoKerning for a style run.

    )pbdoc");

    text_layer.def("set_style_run_auto_kerning", &Class::set_style_run_auto_kerning,
        py::arg("run_index"),
        py::arg("auto_kerning"), R"pbdoc(

        Set /AutoKerning for a style run.

    )pbdoc");

    text_layer.def("style_run_baseline_shift", &Class::style_run_baseline_shift, py::arg("run_index"), R"pbdoc(

        Read /BaselineShift for a style run.

    )pbdoc");

    text_layer.def("set_style_run_baseline_shift", &Class::set_style_run_baseline_shift,
        py::arg("run_index"),
        py::arg("baseline_shift"), R"pbdoc(

        Set /BaselineShift for a style run.

    )pbdoc");

    text_layer.def("style_run_font_caps", &Class::style_run_font_caps, py::arg("run_index"), R"pbdoc(

        Read /FontCaps for a style run.

    )pbdoc");

    text_layer.def("set_style_run_font_caps", &Class::set_style_run_font_caps,
        py::arg("run_index"),
        py::arg("font_caps"), R"pbdoc(

        Set /FontCaps for a style run.

    )pbdoc");

    text_layer.def("style_run_no_break", &Class::style_run_no_break, py::arg("run_index"), R"pbdoc(

        Read /NoBreak for a style run.

    )pbdoc");

    text_layer.def("set_style_run_no_break", &Class::set_style_run_no_break,
        py::arg("run_index"),
        py::arg("no_break"), R"pbdoc(

        Set /NoBreak for a style run.

    )pbdoc");

    text_layer.def("style_run_font_baseline", &Class::style_run_font_baseline, py::arg("run_index"), R"pbdoc(

        Read /FontBaseline for a style run.

    )pbdoc");

    text_layer.def("set_style_run_font_baseline", &Class::set_style_run_font_baseline,
        py::arg("run_index"),
        py::arg("font_baseline"), R"pbdoc(

        Set /FontBaseline for a style run.

    )pbdoc");

    text_layer.def("style_run_language", &Class::style_run_language, py::arg("run_index"), R"pbdoc(

        Read /Language for a style run.

    )pbdoc");

    text_layer.def("set_style_run_language", &Class::set_style_run_language,
        py::arg("run_index"),
        py::arg("language"), R"pbdoc(

        Set /Language for a style run.

    )pbdoc");

    text_layer.def("style_run_character_direction", &Class::style_run_character_direction, py::arg("run_index"), R"pbdoc(

        Read /CharacterDirection for a style run.

    )pbdoc");

    text_layer.def("set_style_run_character_direction", &Class::set_style_run_character_direction,
        py::arg("run_index"),
        py::arg("character_direction"), R"pbdoc(

        Set /CharacterDirection for a style run.

    )pbdoc");

    text_layer.def("style_run_baseline_direction", &Class::style_run_baseline_direction, py::arg("run_index"), R"pbdoc(

        Read /BaselineDirection for a style run.

    )pbdoc");

    text_layer.def("set_style_run_baseline_direction", &Class::set_style_run_baseline_direction,
        py::arg("run_index"),
        py::arg("baseline_direction"), R"pbdoc(

        Set /BaselineDirection for a style run.

    )pbdoc");

    text_layer.def("style_run_tsume", &Class::style_run_tsume, py::arg("run_index"), R"pbdoc(

        Read /Tsume for a style run.

    )pbdoc");

    text_layer.def("set_style_run_tsume", &Class::set_style_run_tsume,
        py::arg("run_index"),
        py::arg("tsume"), R"pbdoc(

        Set /Tsume for a style run.

    )pbdoc");

    text_layer.def("style_run_kashida", &Class::style_run_kashida, py::arg("run_index"), R"pbdoc(

        Read /Kashida for a style run.

    )pbdoc");

    text_layer.def("set_style_run_kashida", &Class::set_style_run_kashida,
        py::arg("run_index"),
        py::arg("kashida"), R"pbdoc(

        Set /Kashida for a style run.

    )pbdoc");

    text_layer.def("style_run_diacritic_pos", &Class::style_run_diacritic_pos, py::arg("run_index"), R"pbdoc(

        Read /DiacriticPos for a style run.

    )pbdoc");

    text_layer.def("set_style_run_diacritic_pos", &Class::set_style_run_diacritic_pos,
        py::arg("run_index"),
        py::arg("diacritic_pos"), R"pbdoc(

        Set /DiacriticPos for a style run.

    )pbdoc");

    text_layer.def("style_run_ligatures", &Class::style_run_ligatures, py::arg("run_index"), R"pbdoc(

        Read /Ligatures for a style run.

    )pbdoc");

    text_layer.def("set_style_run_ligatures", &Class::set_style_run_ligatures,
        py::arg("run_index"),
        py::arg("ligatures"), R"pbdoc(

        Set /Ligatures for a style run.

    )pbdoc");

    text_layer.def("style_run_dligatures", &Class::style_run_dligatures, py::arg("run_index"), R"pbdoc(

        Read /DLigatures for a style run.

    )pbdoc");

    text_layer.def("set_style_run_dligatures", &Class::set_style_run_dligatures,
        py::arg("run_index"),
        py::arg("dligatures"), R"pbdoc(

        Set /DLigatures for a style run.

    )pbdoc");

    text_layer.def("style_run_underline", &Class::style_run_underline, py::arg("run_index"), R"pbdoc(

        Read /Underline for a style run.

    )pbdoc");

    text_layer.def("set_style_run_underline", &Class::set_style_run_underline,
        py::arg("run_index"),
        py::arg("underline"), R"pbdoc(

        Set /Underline for a style run.

    )pbdoc");

    text_layer.def("style_run_strikethrough", &Class::style_run_strikethrough, py::arg("run_index"), R"pbdoc(

        Read /Strikethrough for a style run.

    )pbdoc");

    text_layer.def("set_style_run_strikethrough", &Class::set_style_run_strikethrough,
        py::arg("run_index"),
        py::arg("strikethrough"), R"pbdoc(

        Set /Strikethrough for a style run.

    )pbdoc");

    text_layer.def("style_run_stroke_flag", &Class::style_run_stroke_flag, py::arg("run_index"), R"pbdoc(

        Read /StrokeFlag for a style run.

    )pbdoc");

    text_layer.def("set_style_run_stroke_flag", &Class::set_style_run_stroke_flag,
        py::arg("run_index"),
        py::arg("stroke_flag"), R"pbdoc(

        Set /StrokeFlag for a style run.

    )pbdoc");

    text_layer.def("style_run_fill_flag", &Class::style_run_fill_flag, py::arg("run_index"), R"pbdoc(

        Read /FillFlag for a style run.

    )pbdoc");

    text_layer.def("set_style_run_fill_flag", &Class::set_style_run_fill_flag,
        py::arg("run_index"),
        py::arg("fill_flag"), R"pbdoc(

        Set /FillFlag for a style run.

    )pbdoc");

    text_layer.def("style_run_fill_first", &Class::style_run_fill_first, py::arg("run_index"), R"pbdoc(

        Read /FillFirst for a style run.

    )pbdoc");

    text_layer.def("set_style_run_fill_first", &Class::set_style_run_fill_first,
        py::arg("run_index"),
        py::arg("fill_first"), R"pbdoc(

        Set /FillFirst for a style run.

    )pbdoc");

    text_layer.def("style_run_outline_width", &Class::style_run_outline_width, py::arg("run_index"), R"pbdoc(

        Read /OutlineWidth for a style run.

    )pbdoc");

    text_layer.def("set_style_run_outline_width", &Class::set_style_run_outline_width,
        py::arg("run_index"),
        py::arg("outline_width"), R"pbdoc(

        Set /OutlineWidth for a style run.

    )pbdoc");
}
