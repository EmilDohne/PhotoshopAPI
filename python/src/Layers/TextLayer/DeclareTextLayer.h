#pragma once

// ---------------------------------------------------------------------------
//  DeclareTextLayer.h  --  thin orchestrator
//
//  This header used to be ~2 451 lines.  It is now split into focused
//  sub-headers and this file merely wires them together so that the
//  public API visible from main.cpp is unchanged:
//
//      declare_textlayer_enums(m)
//      declare_text_layer_proxies<T>(m, ext)
//      declare_text_layer<T>(m, ext)
// ---------------------------------------------------------------------------

#include "DeclareTextLayerEnums.h"
#include "DeclareTextLayerProxies.h"

#include "DeclareTextLayerCore.h"
#include "DeclareTextLayerStyleRun.h"
#include "DeclareTextLayerStyleNormal.h"
#include "DeclareTextLayerParagraphRun.h"
#include "DeclareTextLayerParagraphNormal.h"
#include "DeclareTextLayerTransform.h"
#include "DeclareTextLayerRangeStyle.h"

#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"
#include "LayeredFile/LayerTypes/Layer.h"
#include "Implementation/TextLayer.h"
#include "Macros.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <memory>
#include <string>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


// ---------------------------------------------------------------------------
//  declare_textlayer_enums  –  re-exported from DeclareTextLayerEnums.h
//  declare_text_layer_proxies  –  re-exported from DeclareTextLayerProxies.h
// ---------------------------------------------------------------------------
//  (nothing to add – the sub-headers provide them directly)


// ---------------------------------------------------------------------------
//  declare_text_layer<T>  –  creates the PyClass, then delegates to helpers
// ---------------------------------------------------------------------------
template <typename T>
void declare_text_layer(py::module& m, const std::string& extension)
{
    using Class  = TextLayer<T>;
    using PyClass = py::class_<Class, Layer<T>, std::shared_ptr<Class>>;

    // Register proxy types BEFORE the TextLayer class (they reference it)
    declare_character_style_range<T>(m, extension);
    declare_paragraph_style_range<T>(m, extension);

    std::string class_name = "TextLayer" + extension;
    PyClass text_layer(m, class_name.c_str(), py::dynamic_attr(), py::buffer_protocol());

    text_layer.doc() = R"pbdoc(

        A text layer instance. This type is returned when reading PSD/PSB files that
        contain editable text content. Can also be created from scratch by calling
        the constructor directly.

        Attributes
        ----------

        text : Optional[str]
            Read-only property that returns the first detected text payload from the
            layer descriptor, or None if no text payload is present.

    )pbdoc";

    // -- constructor: create from scratch --
    text_layer.def(py::init(&createTextLayer<T>),
        py::arg("layer_name"),
        py::arg("text"),
        py::arg("font") = "ArialMT",
        py::arg("font_size") = 24.0,
        py::arg("fill_color") = std::vector<double>{ 1.0, 0.0, 0.0, 0.0 },
        py::arg("position_x") = 20.0,
        py::arg("position_y") = 50.0,
        py::arg("box_width") = 0.0,
        py::arg("box_height") = 0.0,
        py::arg("layer_mask") = py::none(),
        py::arg("width") = 0,
        py::arg("height") = 0,
        py::arg("blend_mode") = Enum::BlendMode::Normal,
        py::arg("pos_x") = 0,
        py::arg("pos_y") = 0,
        py::arg("opacity") = 1.0f,
        py::arg("compression") = Enum::Compression::ZipPrediction,
        py::arg("color_mode") = Enum::ColorMode::RGB,
        py::arg("is_visible") = true,
        py::arg("is_locked") = false,
        R"pbdoc(

        Construct a new text layer from scratch.

        This constructs a minimal valid text layer with the given text and font settings.
        The layer can be added to a LayeredFile and written to disk. After creating,
        you can use split_style_run() to create sub-ranges and style them independently.

        :param layer_name: Name for the layer, must not exceed 255 characters
        :type layer_name: str

        :param text: The text content (supports multiline via ``\\n``)
        :type text: str

        :param font: PostScript font name. Defaults to "ArialMT"
        :type font: str

        :param font_size: Font size in points. Defaults to 24.0
        :type font_size: float

        :param fill_color: AGBR fill color as 4 float values [0-1]. Defaults to opaque black
        :type fill_color: list[float]

        :param position_x: Horizontal position of the text on the canvas in points. Defaults to 20
        :type position_x: float

        :param position_y: Vertical position (baseline) of the text on the canvas in points. Defaults to 50
        :type position_y: float

        :param box_width: Width of the text box in points. 0 = auto-calculate with generous padding. Defaults to 0
        :type box_width: float

        :param box_height: Height of the text box in points. 0 = auto-calculate with generous padding. Defaults to 0
        :type box_height: float

        :param layer_mask: Optional layer mask, must be a 1D or 2D numpy array of size width * height
        :type layer_mask: numpy.ndarray

        :param width: Width of the layer, only relevant when a mask is provided
        :type width: int

        :param height: Height of the layer, only relevant when a mask is provided
        :type height: int

        :param blend_mode: The blend mode of the layer. Defaults to Normal
        :type blend_mode: psapi.enum.BlendMode

        :param pos_x: Horizontal canvas offset of the layer center. Defaults to 0
        :type pos_x: int

        :param pos_y: Vertical canvas offset of the layer center. Defaults to 0
        :type pos_y: int

        :param opacity: Layer opacity from 0.0 to 1.0. Defaults to 1.0
        :type opacity: float

        :param compression: Compression codec for mask data. Defaults to ZipPrediction
        :type compression: psapi.enum.Compression

        :param color_mode: Color mode, must match the document. Defaults to RGB
        :type color_mode: psapi.enum.ColorMode

        :param is_visible: Whether the layer is visible. Defaults to True
        :type is_visible: bool

        :param is_locked: Whether the layer is locked. Defaults to False
        :type is_locked: bool

        :raises:
            ValueError: if layer_name exceeds 255 characters

            ValueError: if layer_mask size does not match width * height

            ValueError: if width or height is negative

            ValueError: if opacity is not between 0 and 1

    )pbdoc");

    // -- split_style_run --
    text_layer.def("split_style_run", &Class::split_style_run,
        py::arg("run_index"),
        py::arg("char_offset"),
        R"pbdoc(

        Split a style run at the given character offset.

        After splitting, the original run covers characters [0, char_offset) and
        a new run is inserted covering [char_offset, end_of_run). The new run
        inherits all style properties from the original and can be styled independently.

        This is the key method for applying different styles to different parts of text.
        For example, to bold the word "World" in "Hello World":

        .. code-block:: python

            layer = psapi.TextLayer_8bit("my_text", "Hello World")
            layer.split_style_run(0, 6)  # split at offset 6 -> "Hello " | "World"
            run1 = layer.style_run(1)
            run1.set_faux_bold(True)

        :param run_index: Zero-based index of the run to split
        :type run_index: int

        :param char_offset: UTF-16 code-unit offset within the run where the split occurs
        :type char_offset: int

        :raises:
            RuntimeError: if no TySh descriptor exists or the split cannot be applied

    )pbdoc");

    // -- style_run_lengths --
    text_layer.def("style_run_lengths", &Class::style_run_lengths,
        R"pbdoc(

        Get the list of style run lengths (in UTF-16 code units).

        Each element corresponds to a style run. The sum of all lengths equals
        the total text length (including the trailing CR used by EngineData).

        :return: List of run lengths, or None if unavailable
        :rtype: Optional[list[int]]

    )pbdoc");

    // -- wire up every section --
    bind_textlayer_core_apis<T>(text_layer);
    bind_textlayer_style_run_apis<T>(text_layer);
    bind_textlayer_style_normal_apis<T>(text_layer);
    bind_textlayer_paragraph_run_apis<T>(text_layer);
    bind_textlayer_paragraph_normal_apis<T>(text_layer);
    bind_textlayer_transform_apis<T>(text_layer);
    bind_textlayer_range_style_apis<T>(text_layer);
}
