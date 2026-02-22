#pragma once

#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"
#include "LayeredFile/LayerTypes/Layer.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>
#include <optional>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


// ---------------------------------------------------------------------------
//  Transform, warp, position, and proxy accessor bindings
// ---------------------------------------------------------------------------

template <typename T>
void bind_textlayer_transform_apis(py::class_<TextLayer<T>, Layer<T>, std::shared_ptr<TextLayer<T>>>& text_layer)
{
    using Class = TextLayer<T>;

    // -----------------------------------------------------------------------
    //  Transform Read/Write APIs
    // -----------------------------------------------------------------------

    text_layer.def("transform", &Class::transform, R"pbdoc(

        Read the six affine-transform doubles from the TySh header.
        Returns a list [xx, xy, yx, yy, tx, ty].  Empty list if no TySh block found.

    )pbdoc");

    text_layer.def("transform_component", &Class::transform_component, py::arg("index"), R"pbdoc(

        Read a single transform component by index (0–5).
        Index mapping: 0=xx, 1=xy, 2=yx, 3=yy, 4=tx, 5=ty.
        Returns None for out-of-range index or missing TySh.

    )pbdoc");

    text_layer.def_property_readonly("transform_xx", &Class::transform_xx, R"pbdoc(
        Horizontal scale component (xx) of the affine transform.
    )pbdoc");
    text_layer.def_property_readonly("transform_xy", &Class::transform_xy, R"pbdoc(
        Horizontal shear component (xy) of the affine transform.
    )pbdoc");
    text_layer.def_property_readonly("transform_yx", &Class::transform_yx, R"pbdoc(
        Vertical shear component (yx) of the affine transform.
    )pbdoc");
    text_layer.def_property_readonly("transform_yy", &Class::transform_yy, R"pbdoc(
        Vertical scale component (yy) of the affine transform.
    )pbdoc");
    text_layer.def_property_readonly("transform_tx", &Class::transform_tx, R"pbdoc(
        Horizontal translation (tx) of the affine transform.
    )pbdoc");
    text_layer.def_property_readonly("transform_ty", &Class::transform_ty, R"pbdoc(
        Vertical translation (ty) of the affine transform.
    )pbdoc");

    text_layer.def("set_transform", &Class::set_transform, py::arg("values"), R"pbdoc(

        Write all six affine-transform doubles into the TySh header.
        ``values`` must be a list of exactly 6 floats [xx, xy, yx, yy, tx, ty].
        :raises ValueError: if values does not contain exactly 6 numbers
        :raises RuntimeError: if no writable TySh transform payload exists

    )pbdoc");

    text_layer.def("set_transform_component", &Class::set_transform_component,
        py::arg("index"), py::arg("value"), R"pbdoc(

        Write a single transform component by index (0–5).
        :raises ValueError: if index is outside [0, 5]
        :raises RuntimeError: if no writable TySh transform payload exists

    )pbdoc");

    text_layer.def("set_transform_xx", &Class::set_transform_xx, py::arg("value"), R"pbdoc(
        Set horizontal scale component (xx).
    )pbdoc");
    text_layer.def("set_transform_xy", &Class::set_transform_xy, py::arg("value"), R"pbdoc(
        Set horizontal shear component (xy).
    )pbdoc");
    text_layer.def("set_transform_yx", &Class::set_transform_yx, py::arg("value"), R"pbdoc(
        Set vertical shear component (yx).
    )pbdoc");
    text_layer.def("set_transform_yy", &Class::set_transform_yy, py::arg("value"), R"pbdoc(
        Set vertical scale component (yy).
    )pbdoc");
    text_layer.def("set_transform_tx", &Class::set_transform_tx, py::arg("value"), R"pbdoc(
        Set horizontal translation (tx).
    )pbdoc");
    text_layer.def("set_transform_ty", &Class::set_transform_ty, py::arg("value"), R"pbdoc(
        Set vertical translation (ty).
    )pbdoc");

    // -----------------------------------------------------------------------
    //  Transform Convenience APIs (rotation / scale)
    // -----------------------------------------------------------------------

    text_layer.def_property_readonly("rotation_angle", &Class::rotation_angle, R"pbdoc(
        Current rotation angle in degrees, derived from the affine transform
        matrix via ``atan2(xy, xx)``.
    )pbdoc");

    text_layer.def_property_readonly("scale_x", &Class::scale_x, R"pbdoc(
        Horizontal scale factor derived from the transform matrix
        (``sqrt(xx² + xy²)``).  1.0 means 100 %.
    )pbdoc");

    text_layer.def_property_readonly("scale_y", &Class::scale_y, R"pbdoc(
        Vertical scale factor derived from the transform matrix
        (``sqrt(yx² + yy²)``).  1.0 means 100 %.
    )pbdoc");

    text_layer.def("set_rotation_angle", &Class::set_rotation_angle,
        py::arg("angle_degrees"), R"pbdoc(
        Rotate the text layer to the given angle (in degrees) while
        preserving the existing scale factors and translation.
    )pbdoc");

    text_layer.def("set_scale_x", &Class::set_scale_x,
        py::arg("sx"), R"pbdoc(
        Set the horizontal scale factor while preserving the current
        rotation angle and translation.
        :raises ValueError: when the current scale is degenerate (near-zero)
        :raises RuntimeError: if transform data is unavailable
    )pbdoc");

    text_layer.def("set_scale_y", &Class::set_scale_y,
        py::arg("sy"), R"pbdoc(
        Set the vertical scale factor while preserving the current
        rotation angle and translation.
        :raises ValueError: when the current scale is degenerate (near-zero)
        :raises RuntimeError: if transform data is unavailable
    )pbdoc");

    text_layer.def("set_scale", &Class::set_scale,
        py::arg("sx"), py::arg("sy") = py::none(), R"pbdoc(
        Set scale factor(s) while preserving the current rotation angle
        and translation.

        - ``set_scale(1.5)`` — uniform scale (same X and Y).
        - ``set_scale(0.86, 1.18)`` — non-uniform scale.

        :raises ValueError: when the current scale is degenerate (near-zero)
        :raises RuntimeError: if transform data is unavailable
    )pbdoc");

    // Python wrapper: when sy is None, treat as uniform scaling
    // We override the binding with a lambda that fills in sy = sx when omitted.
    text_layer.def("set_scale",
        [](Class& self, double sx, std::optional<double> sy) {
            self.set_scale(sx, sy.value_or(sx));
        },
        py::arg("sx"), py::arg("sy") = py::none(), R"pbdoc(
        Set scale factor(s) while preserving the current rotation angle
        and translation.

        - ``set_scale(1.5)`` — uniform scale (same X and Y).
        - ``set_scale(0.86, 1.18)`` — non-uniform scale.

        :raises ValueError: when the current scale is degenerate (near-zero)
        :raises RuntimeError: if transform data is unavailable
    )pbdoc");

    // -----------------------------------------------------------------------
    //  Position convenience APIs
    // -----------------------------------------------------------------------

    text_layer.def("position", [](const Class& self) {
            auto p = self.position();
            return py::make_tuple(p.first, p.second);
        }, R"pbdoc(
        Return the translation (position) as a tuple ``(tx, ty)``.
    )pbdoc");

    text_layer.def("set_position", &Class::set_position,
        py::arg("x"), py::arg("y"), R"pbdoc(
        Set the translation (position) to ``(x, y)``, leaving rotation
        and scale unchanged.
    )pbdoc");

    text_layer.def("reset_transform", &Class::reset_transform, R"pbdoc(
        Reset rotation and scale to identity while preserving the current
        translation (position).  After this call the transform is
        ``[1, 0, 0, 1, tx, ty]``.
    )pbdoc");

    // -----------------------------------------------------------------------
    //  High-level font convenience APIs
    // -----------------------------------------------------------------------

    text_layer.def_property_readonly("primary_font_name", &Class::primary_font_name, R"pbdoc(
        PostScript name of the font used by the first style run (the
        "primary" font).  Returns None when unavailable.
    )pbdoc");

    text_layer.def("set_font", &Class::set_font,
        py::arg("postscript_name"), R"pbdoc(
        Set a single font across the entire text layer — all style runs
        and the normal style sheet.  The font is looked up by PostScript
        name; if it does not yet exist in the FontSet it is added
        automatically.
    )pbdoc");

    // -----------------------------------------------------------------------
    //  Warp Read APIs
    // -----------------------------------------------------------------------

    text_layer.def_property_readonly("has_warp", &Class::has_warp, R"pbdoc(

        True when a non-trivial warp is applied (warpStyle != "warpNone").

    )pbdoc");

    text_layer.def_property_readonly("warp_style", &Class::warp_style, R"pbdoc(

        Warp style enum from the TySh warp descriptor.
        Returns a WarpStyle value (e.g. WarpStyle.Arc, WarpStyle.None).
        Returns None when unavailable.

    )pbdoc");

    text_layer.def_property_readonly("warp_value", &Class::warp_value, R"pbdoc(

        Warp bend amount (warpValue) from the TySh warp descriptor.
        Typically in [-100, 100].  Returns None when unavailable.

    )pbdoc");

    text_layer.def_property_readonly("warp_horizontal_distortion", &Class::warp_horizontal_distortion, R"pbdoc(

        Horizontal distortion (warpPerspective) from the TySh warp descriptor.
        Typically in [-100, 100].  Returns None when unavailable.

    )pbdoc");

    text_layer.def_property_readonly("warp_vertical_distortion", &Class::warp_vertical_distortion, R"pbdoc(

        Vertical distortion (warpPerspectiveOther) from the TySh warp descriptor.
        Typically in [-100, 100].  Returns None when unavailable.

    )pbdoc");

    text_layer.def_property_readonly("warp_rotation", &Class::warp_rotation, R"pbdoc(

        Warp orientation: 0 = horizontal, 1 = vertical.
        Returns None when unavailable.

    )pbdoc");

    // -----------------------------------------------------------------------
    //  Proxy accessor methods
    // -----------------------------------------------------------------------

    text_layer.def("style_run", &Class::style_run,
        py::arg("run_index"), R"pbdoc(

        Return a proxy object for the character style run at the given index.

        The proxy provides grouped read/write access to all style-run properties
        (font_size, faux_bold, fill_color, etc.) without needing to call the
        flat ``style_run_*`` functions directly.

        .. warning::
            The proxy is a lightweight non-owning handle.  It must not be
            stored beyond the lifetime of the parent TextLayer.

    )pbdoc");

    text_layer.def("style_normal", &Class::style_normal, R"pbdoc(

        Return a proxy object for the normal (default) character style sheet.

        The proxy provides grouped read/write access to all normal-style
        properties (font, font_size, faux_bold, fill_color, etc.).

        .. warning::
            The proxy is a lightweight non-owning handle.  It must not be
            stored beyond the lifetime of the parent TextLayer.

    )pbdoc");

    text_layer.def("paragraph_run", &Class::paragraph_run,
        py::arg("run_index"), R"pbdoc(

        Return a proxy object for the paragraph run at the given index.

        The proxy provides grouped read/write access to all paragraph-run
        properties (justification, auto_hyphenate, word_spacing, etc.).

        .. warning::
            The proxy is a lightweight non-owning handle.  It must not be
            stored beyond the lifetime of the parent TextLayer.

    )pbdoc");

    text_layer.def("paragraph_normal", &Class::paragraph_normal, R"pbdoc(

        Return a proxy object for the normal (default) paragraph sheet.

        The proxy provides grouped read/write access to all normal-paragraph
        properties (justification, auto_hyphenate, word_spacing, etc.).

        .. warning::
            The proxy is a lightweight non-owning handle.  It must not be
            stored beyond the lifetime of the parent TextLayer.

    )pbdoc");

    text_layer.def("font", &Class::font,
        py::arg("font_index"), R"pbdoc(

        Return a proxy object for the font at the given FontSet index.

        The proxy exposes postscript_name, script, type, synthetic, and
        is_sentinel, plus set_postscript_name().

        .. warning::
            The proxy is a lightweight non-owning handle.  It must not be
            stored beyond the lifetime of the parent TextLayer.

    )pbdoc");

    text_layer.def("font_set", &Class::font_set, R"pbdoc(

        Return a proxy object for the FontSet as a whole.

        The proxy exposes count, used_indices, used_names, find_index(),
        and add().

        .. warning::
            The proxy is a lightweight non-owning handle.  It must not be
            stored beyond the lifetime of the parent TextLayer.

    )pbdoc");
}
