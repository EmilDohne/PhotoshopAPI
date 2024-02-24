#include "LayeredFile/LayerTypes/GroupLayer.h"
#include "LayeredFile/LayerTypes/Layer.h"
#include "Macros.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>
#include <pybind11/iostream.h>

#include <memory>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;

// Create an alternative constructor which inlines the Layer<T>::Params since the more pythonic version would be to have kwargs rather 
// than a separate structure as well as creating an interface for numpy
template <typename T>
std::shared_ptr<GroupLayer<T>> createGroupLayer(
    const std::string& layer_name,
    const std::optional<py::array_t<T>> layer_mask,
    int width,  // This is only relevant if a layer mask is set
    int height, // This is only relevant if a layer mask is set
    const Enum::BlendMode blend_mode,
    int pos_x, // This is only relevant if a layer mask is set
    int pos_y, // This is only relevant if a layer mask is set
    int opacity,
    const Enum::Compression compression,
    const Enum::ColorMode color_mode,
    bool is_collapsed
)
{
    typename Layer<T>::Params params;
    // Do some preliminary checks since python has no concept of e.g. unsigned integers (without ctypes) 
    // so we must ensure the range ourselves
    if (layer_name.size() > 255)
    {
        throw py::value_error("layer_name parameter cannot exceed a length of 255");
    }
    if (layer_mask.has_value())
    {
        if (static_cast<uint64_t>(width) * height != layer_mask.value().size())
        {
            throw py::value_error("layer_mask parameter must have the same size as the layer itself (width * height)");
        }
        params.layerMask = std::vector<T>(layer_mask.value().data(), layer_mask.value().data() + layer_mask.value().size());
    }
    if (width < 0)
    {
        throw py::value_error("width cannot be a negative value");
    }
    if (height < 0)
    {
        throw py::value_error("height cannot be a negative value");
    }
    if (opacity < 0 || opacity > 255)
    {
        throw py::value_error("opacity must be between 0-255 where 255 is 100%, got " + std::to_string(opacity));
    }

    params.layerName = layer_name;
    params.blendMode = blend_mode;
    params.posX = pos_x;
    params.posY = pos_y;
    params.width = width;
    params.height = height;
    params.opacity = opacity;
    params.compression = compression;
    params.colorMode = color_mode;
    return std::make_shared<GroupLayer<T>>(params, is_collapsed);
}


// Generate a LayeredFile python class from our struct adjusting some
// of the methods 
template <typename T>
void declareGroupLayer(py::module& m, const std::string& extension) {
    using Class = GroupLayer<T>;
    std::string className = "GroupLayer" + extension;
    py::class_<Class, Layer<T>, std::shared_ptr<Class>> groupLayer(m, className.c_str(), py::dynamic_attr());

    groupLayer.def(py::init(&createGroupLayer<T>),
        py::arg("layer_name"),
        py::arg("layer_mask"),
        py::arg("width") = 0,
        py::arg("height") = 0,
        py::arg("blend_mode") = Enum::BlendMode::Passthrough,
        py::arg("pos_x") = 0,
        py::arg("pos_y") = 0,
        py::arg("opacity") = 255,
        py::arg("compression") = Enum::Compression::ZipPrediction,
        py::arg("color_mode") = Enum::ColorMode::RGB,
        py::arg("is_collapsed") = false);

    groupLayer.def_readwrite("layers", &Class::m_Layers);
    groupLayer.def_readwrite("is_collapsed", &Class::m_isCollapsed);

    groupLayer.def("add_layer", &Class::addLayer, py::arg("layered_file"), py::arg("layer"));

    groupLayer.def("remove_layer", py::overload_cast<const int>(&Class::removeLayer), py::arg("index"));
    groupLayer.def("remove_layer", py::overload_cast<std::shared_ptr<Layer<T>>&>(&Class::removeLayer), py::arg("layer"));
    groupLayer.def("remove_layer", py::overload_cast<const std::string>(&Class::removeLayer), py::arg("layer_name"));

    // Implement dict-like indexing for group layers with the [] syntax
    groupLayer.def("__getitem__", [](const Class& self, const std::string value)
        {
            for (auto& layer : self.m_Layers)
            {
                // Check if the layers are the same
                if (layer->m_LayerName == value)
                {   
                    // pybind automatically downcasts this to the appropriate type so no action is needed
                    return layer;
                }
            }
            throw py::key_error("Unable to find layer '" + value + "' in the Group");
        } py::arg("value"));
}