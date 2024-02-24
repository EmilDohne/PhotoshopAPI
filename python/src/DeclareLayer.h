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
#include <optional>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


// Generate a LayeredFile python class from our struct adjusting some
// of the methods 
template <typename T>
void declareLayer(py::module& m, const std::string& extension) {
    using Class = Layer<T>;
    std::string className = "Layer" + extension;
    // Designate shared_ptr as the holder type so pybind11 doesnt cast to unique_ptr giving us a heap corruption error
    py::class_<Class, std::shared_ptr<Class>> layer(m, className.c_str(), py::dynamic_attr(), py::buffer_protocol());

    layer.def_readwrite("name", &Class::m_LayerName);
    layer.def_property_readonly("layer_mask", [](Class& self)
        {
            std::vector<T> data = self.getMaskData();
            if (data.size() != 0)
            {
                // If the size is greater than 0 that means we have a maskchannel
                // which is why we can access .value() directly
                auto width = static_cast<size_t>(self.m_LayerMask.value().maskData.getWidth());
                auto height = static_cast<size_t>(self.m_LayerMask.value().maskData.getHeight());

                // Get pointer to copied data and size
                T* ptr = data.data();
                std::vector<size_t> shape = { height, width };

                return py::array_t<T>(shape, ptr);
            }
            return py::array_t<T>(0, nullptr)
        });
    layer.def_readwrite("blend_mode", &Class::m_BlendMode);
    layer.def_readwrite("is_visible", &Class::m_IsVisible);
    layer.def_readwrite("opacity", &Class::m_Opacity);
    layer.def_readwrite("width", &Class::m_Width);
    layer.def_readwrite("height", &Class::m_Height);
    layer.def_readwrite("center_x", &Class::m_CenterX);
    layer.def_readwrite("center_y", &Class::m_CenterY);

    layer.def("get_mask_data", [](Class& self, const bool do_copy)
        {
            std::vector<T> data = self.getMaskData(do_copy);
            if (data.size() != 0)
            { 
                // If the size is greater than 0 that means we have a maskchannel
                // which is why we can access .value() directly
                auto width = static_cast<size_t>(self.m_LayerMask.value().maskData.getWidth());
                auto height = static_cast<size_t>(self.m_LayerMask.value().maskData.getHeight());

                // Get pointer to copied data and size
                T* ptr = data.data();
                std::vector<size_t> shape = { height, width};

                return py::array_t<T>(shape, ptr);
            }
            return py::array_t<T>(0, nullptr);
        });
}