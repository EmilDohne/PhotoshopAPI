#pragma once

#include "Util/Enum.h"
#include "LayeredFile/LayerTypes/MaskDataMixin.h"
#include "PyUtil/ImageConversion.h"
#include "Macros.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>
#include <pybind11/iostream.h>

#include <unordered_map>
#include <iostream>
#include <vector>
#include <span>


namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


template <typename T, typename Class, typename PyClass>
    requires std::is_base_of_v<MaskMixin<T>, Class> && std::is_base_of_v<Layer<T>, Class>
void bind_mask_mixin(PyClass& bound_class)
{
    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    bound_class.def("has_mask", [](Class& self) { return self.has_mask(); }, R"pbdoc(

        Check whether the layer has an associated mask component (pixel mask)

	)pbdoc");


    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    bound_class.def_property("mask", [](Class& self)
        {
            if (self.has_mask())
            {
                std::vector<T> data = self.get_mask();
                return to_py_array(std::move(data), self.mask_width(), self.mask_height());
            }
            return py::array_t<T>();
        }, [](Class& self, py::array_t<T>& data)
        {
            // Our mask setting allows for two overloads, one with an explicit width and height and another
            // without any additional arguments. Here we make the assumption/simplification that the user will
            // always pass in a 2D array allowing us to get the width and height from it. This gives us a better
            // user interface for setting the new mask dimensions
            auto shape = Util::Impl::shape_from_py_array(data, { 2 }, data.size());
            auto view = from_py_array(tag::view{}, data, shape[1], shape[0]);
            self.set_mask(view, shape[1], shape[0]);

        }, R"pbdoc(

        The layers' pixel mask, this is a 2-dimensional array stored in format { height, width }. A pixel mask may have
        any dimensions and does not have to match a layers' width or height. 
        To get the pixel value outside of the masks' bbox use the mask_default_color property.

	)pbdoc");


    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    bound_class.def_property("mask_disabled", [](Class& self) { return self.mask_disabled(); }, [](Class& self, bool value) { self.mask_disabled(value); }, R"pbdoc(

        Whether the mask is disabled. Ignored if no mask is present

	)pbdoc");

    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    bound_class.def_property("mask_relative_to_layer", [](Class& self) { return self.mask_relative_to_layer(); }, [](Class& self, bool value) { self.mask_relative_to_layer(value); }, R"pbdoc(

        Whether the masks position is relative to the layer. Ignored if no mask is present

	)pbdoc");

    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    bound_class.def_property("mask_default_color", [](Class& self) { return self.mask_default_color(); }, [](Class& self, uint8_t value) { self.mask_default_color(value); }, R"pbdoc(

        The masks' default color outside of the masks bounding box. Ignored if no mask is present.
        From 0-255 regardless of bit depth

	)pbdoc");

    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    bound_class.def_property("mask_density", [](Class& self) { return self.mask_density(); }, [](Class& self, uint8_t value) { self.mask_density(value); }, R"pbdoc(

        Optional mask density from 0-255, this is equivalent to layers' opacity. Ignored if no mask is present

	)pbdoc");

    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    bound_class.def_property("mask_feather", [](Class& self) { return self.mask_feather(); }, [](Class& self, float64_t value) { self.mask_feather(value); }, R"pbdoc(

        Optional mask feather. Ignored if no mask is present

	)pbdoc");

    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    bound_class.def_property("mask_position", [](Class& self) { return self.mask_position(); }, [](Class& self, Geometry::Point2D<double> value) { self.mask_position(value); }, R"pbdoc(

        The masks' canvas coordinates, these represent the center of the mask in terms of the canvas (file). Ignored if no mask is present

	)pbdoc");

    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    bound_class.def("mask_width", &Class::mask_width, R"pbdoc(

        The masks' width in pixels. This does not always have to correspond with the layers' width.

	)pbdoc");

    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    bound_class.def("mask_height", &Class::mask_height, R"pbdoc(

        The masks' height in pixels. This does not always have to correspond with the layers' height.

	)pbdoc");

    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    bound_class.def("set_mask_compression", [](Class& self, Enum::Compression compcode) { self.set_mask_compression(compcode); }, R"pbdoc(

        Set the masks' write compression in terms of one of the Photoshop compression codecs. The mask channel may have any compression codec
        applied to it and this does not need to match the layers' compression in any way. All compression codecs are valid in the PhotoshopAPI.

	)pbdoc");
}