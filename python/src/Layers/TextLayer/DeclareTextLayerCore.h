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
//  Core TextLayer bindings: text content, font set, orientation, shape, counts
// ---------------------------------------------------------------------------

template <typename T>
void bind_textlayer_core_apis(py::class_<TextLayer<T>, Layer<T>, std::shared_ptr<TextLayer<T>>>& text_layer)
{
    using Class = TextLayer<T>;

    text_layer.def_property_readonly("text", &Class::text, R"pbdoc(

        Read the current text payload from the layer.

    )pbdoc");

    text_layer.def("set_text", &Class::set_text, py::arg("value"), R"pbdoc(

        Replace the full text payload with a new value.

        Returns True when a payload was found and updated.

    )pbdoc");

    text_layer.def("replace_text", &Class::replace_text,
        py::arg("old_text"),
        py::arg("new_text"),
        py::arg("replace_all") = true, R"pbdoc(

        Replace text occurrences in the payload.

        Returns True when at least one occurrence was replaced.


    )pbdoc");

    text_layer.def("set_text_equal_length", &Class::set_text_equal_length, py::arg("value"), R"pbdoc(

        Replace the full text payload only if UTF-16 code-unit length matches.

    )pbdoc");

    text_layer.def("replace_text_equal_length", &Class::replace_text_equal_length,
        py::arg("old_text"),
        py::arg("new_text"),
        py::arg("replace_all") = true, R"pbdoc(

        Replace text occurrences only when old and new UTF-16 code-unit lengths match.

    )pbdoc");

    // ================================================================
    // FontSet read APIs
    // ================================================================

    text_layer.def_property_readonly("font_count", &Class::font_count, R"pbdoc(

        Number of fonts in the EngineData /ResourceDict /FontSet array.

    )pbdoc");

    text_layer.def("font_postscript_name", &Class::font_postscript_name, py::arg("font_index"), R"pbdoc(

        Retrieve the PostScript name of the font at the given index.

        The index corresponds to the /Font integer stored in style run and normal style sheet data.

    )pbdoc");

    text_layer.def("font_name", &Class::font_name, py::arg("font_index"), R"pbdoc(

        Convenience alias for font_postscript_name().

    )pbdoc");

    text_layer.def("font_script", &Class::font_script, py::arg("font_index"), R"pbdoc(

        Retrieve the /Script value for the font at the given index (0 = Roman, 1 = CJK, etc.).

    )pbdoc");

    text_layer.def("font_type", &Class::font_type, py::arg("font_index"), R"pbdoc(

        Retrieve the /FontType value for the font at the given index (0 = OpenType, 1 = TrueType, etc.).

    )pbdoc");

    text_layer.def("font_synthetic", &Class::font_synthetic, py::arg("font_index"), R"pbdoc(

        Retrieve the /Synthetic flag for the font at the given index.
        Non-zero indicates a synthetic (missing) font substitute.

    )pbdoc");

    // -----------------------------------------------------------------------
    //  Missing-font query helpers
    // -----------------------------------------------------------------------

    text_layer.def("is_sentinel_font", &Class::is_sentinel_font, py::arg("font_index"), R"pbdoc(

        Return True when the font at the given index is Photoshop's internal
        invisible sentinel font (AdobeInvisFont).

    )pbdoc");

    text_layer.def("used_font_indices", &Class::used_font_indices, R"pbdoc(

        Return the distinct sorted list of FontSet indices actually referenced by
        style runs.

    )pbdoc");

    text_layer.def("used_font_names", &Class::used_font_names, R"pbdoc(

        Return the distinct PostScript names of fonts actually referenced by
        style runs, excluding the AdobeInvisFont sentinel.  These are the
        "real" fonts the text layer needs to render correctly.

    )pbdoc");

    // -----------------------------------------------------------------------
    //  FontSet write / sync
    // -----------------------------------------------------------------------

    text_layer.def("add_font", &Class::add_font, py::arg("postscript_name"),
        py::arg("font_type") = TextLayerEnum::FontType::OpenType, py::arg("script") = TextLayerEnum::FontScript::Roman, py::arg("synthetic") = 0, R"pbdoc(

        Add a new font entry to the FontSet (both ResourceDict and DocumentResources).
        Returns the zero-based index of the new entry, or -1 on failure.

    )pbdoc");

    text_layer.def("set_font_postscript_name", &Class::set_font_postscript_name,
        py::arg("font_index"), py::arg("new_name"), R"pbdoc(

        Rename an existing font entry in both ResourceDict/FontSet and
        DocumentResources/FontSet.  Returns True on success.

    )pbdoc");

    text_layer.def("find_font_index", &Class::find_font_index,
        py::arg("postscript_name"), R"pbdoc(

        Find the index of a font in the FontSet by PostScript name.
        Returns the zero-based index, or -1 if not found.

    )pbdoc");

    text_layer.def("set_style_run_font_by_name", &Class::set_style_run_font_by_name,
        py::arg("run_index"), py::arg("postscript_name"), R"pbdoc(

        Find-or-add the font by PostScript name, then set it as the /Font
        for the given style run.  Returns True on success.

    )pbdoc");

    text_layer.def("set_style_normal_font_by_name", &Class::set_style_normal_font_by_name,
        py::arg("postscript_name"), R"pbdoc(

        Find-or-add the font by PostScript name, then set it as the /Font
        in the normal style sheet.  Returns True on success.

    )pbdoc");

    // -----------------------------------------------------------------------
    //  Orientation / WritingDirection
    // -----------------------------------------------------------------------

    text_layer.def("orientation", &Class::orientation, R"pbdoc(

        Return the WritingDirection enum from EngineData.
        Returns None when the value cannot be read.

    )pbdoc");

    text_layer.def_property_readonly("is_vertical", &Class::is_vertical, R"pbdoc(

        True when the text layer uses vertical writing direction (WritingDirection == 2).

    )pbdoc");

    text_layer.def("set_orientation", &Class::set_orientation, py::arg("writing_direction"), R"pbdoc(

        Set the WritingDirection enum in the EngineData.
        Use WritingDirection.Horizontal or WritingDirection.Vertical.
        Returns True on success.

    )pbdoc");

    // -----------------------------------------------------------------------
    //  Text Frame / Shape (point text vs box text)
    // -----------------------------------------------------------------------

    text_layer.def("shape_type", &Class::shape_type, R"pbdoc(

        Return the ShapeType enum from EngineData.
        Returns None when unavailable.

    )pbdoc");

    text_layer.def_property_readonly("is_box_text", &Class::is_box_text, R"pbdoc(

        True when the text layer is area (box) text (ShapeType == 1).

    )pbdoc");

    text_layer.def_property_readonly("is_point_text", &Class::is_point_text, R"pbdoc(

        True when the text layer is point text (ShapeType == 0).

    )pbdoc");

    text_layer.def("box_bounds", &Class::box_bounds, R"pbdoc(

        Return the box bounds as [top, left, bottom, right] in text-space coordinates.
        Returns None for point text or if the data cannot be read.

    )pbdoc");

    text_layer.def("box_width", &Class::box_width, R"pbdoc(

        Return the box width (right - left), or None for point text.

    )pbdoc");

    text_layer.def("box_height", &Class::box_height, R"pbdoc(

        Return the box height (bottom - top), or None for point text.

    )pbdoc");

    text_layer.def("set_box_bounds", &Class::set_box_bounds,
        py::arg("top"), py::arg("left"), py::arg("bottom"), py::arg("right"), R"pbdoc(

        Set the box bounds in text-space coordinates.
        The layer must already be box text.  Returns True on success.

    )pbdoc");

    text_layer.def("set_box_size", &Class::set_box_size,
        py::arg("width"), py::arg("height"), R"pbdoc(

        Set box width and height while keeping the current top-left corner.
        The layer must already be box text.  Returns True on success.

    )pbdoc");

    text_layer.def("set_box_width", &Class::set_box_width,
        py::arg("width"), R"pbdoc(

        Set only the box width, keeping top, left, and height unchanged.
        The layer must already be box text.  Returns True on success.

    )pbdoc");

    text_layer.def("set_box_height", &Class::set_box_height,
        py::arg("height"), R"pbdoc(

        Set only the box height, keeping top, left, and width unchanged.
        The layer must already be box text.  Returns True on success.

    )pbdoc");

    text_layer.def("convert_to_box_text", &Class::convert_to_box_text,
        py::arg("width"), py::arg("height"), R"pbdoc(

        Convert a point-text layer to box (area) text with the given dimensions.
        The box is placed at (0, 0) in text-space coordinates.
        Returns False if the layer is already box text.

    )pbdoc");

    text_layer.def("convert_to_point_text", &Class::convert_to_point_text, R"pbdoc(

        Convert a box-text layer back to point text, removing the bounding rectangle.
        Returns False if the layer is already point text.

    )pbdoc");

    // -----------------------------------------------------------------------
    //  Anti-aliasing
    // -----------------------------------------------------------------------

    text_layer.def("anti_alias", &Class::anti_alias, R"pbdoc(

        Return the anti-aliasing method for this text layer.
        Returns None when the value cannot be read (e.g. no TySh descriptor).

        :rtype: Optional[AntiAliasMethod]

    )pbdoc");

    text_layer.def("set_anti_alias", &Class::set_anti_alias, py::arg("method"), R"pbdoc(

        Set the anti-aliasing method for this text layer.

        :param method: The anti-aliasing method to apply
        :type method: AntiAliasMethod
        :raises:
            ValueError: if an unsupported anti-alias enum value is passed
            RuntimeError: if this layer has no TySh tagged block or writing AntA fails

    )pbdoc");

    // -----------------------------------------------------------------------
    //  Run / sheet counts
    // -----------------------------------------------------------------------

    text_layer.def_property_readonly("style_run_count", &Class::style_run_count, R"pbdoc(

        Number of style runs found in the EngineData /StyleRun /RunArray section.

    )pbdoc");

    text_layer.def_property_readonly("paragraph_run_count", &Class::paragraph_run_count, R"pbdoc(

        Number of paragraph runs found in the EngineData /ParagraphRun /RunArray section.

    )pbdoc");
}
