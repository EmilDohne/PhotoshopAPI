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

#include "TextLayer/DeclareTextLayerEnums.h"
#include "TextLayer/DeclareTextLayerProxies.h"

#include "TextLayer/DeclareTextLayerCore.h"
#include "TextLayer/DeclareTextLayerStyleRun.h"
#include "TextLayer/DeclareTextLayerStyleNormal.h"
#include "TextLayer/DeclareTextLayerParagraphRun.h"
#include "TextLayer/DeclareTextLayerParagraphNormal.h"
#include "TextLayer/DeclareTextLayerTransform.h"

#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"
#include "LayeredFile/LayerTypes/Layer.h"
#include "Macros.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

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

    std::string class_name = "TextLayer" + extension;
    PyClass text_layer(m, class_name.c_str(), py::dynamic_attr(), py::buffer_protocol());

    text_layer.doc() = R"pbdoc(

        A text layer instance. This type is returned when reading PSD/PSB files that
        contain editable text content.

        Attributes
        ----------

        text : Optional[str]
            Read-only property that returns the first detected text payload from the
            layer descriptor, or None if no text payload is present.

    )pbdoc";

    // -- wire up every section --
    bind_textlayer_core_apis<T>(text_layer);
    bind_textlayer_style_run_apis<T>(text_layer);
    bind_textlayer_style_normal_apis<T>(text_layer);
    bind_textlayer_paragraph_run_apis<T>(text_layer);
    bind_textlayer_paragraph_normal_apis<T>(text_layer);
    bind_textlayer_transform_apis<T>(text_layer);
}
