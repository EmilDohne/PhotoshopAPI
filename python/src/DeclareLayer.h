#include "LayeredFile/LayerTypes/Layer.h"
#include "Macros.h"
#include "PyUtil/ImageConversion.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>
#include <pybind11/iostream.h>

#include <memory>
#include <optional>

// If we compile with C++<20 we replace the stdlib implementation with the compatibility
// library
#if (__cplusplus < 202002L)
#include "tcb_span.hpp"
#else
#include <span>
#endif

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
    layer.doc() = R"pbdoc(

        Base type that all layers inherit from, this class should not be instantiated
        and instead the derivatives such as :class:`psapi.GroupLayer_8bit` or :class:`psapi.ImageLayer_8bit`
        should be used (with the appropriate bit depth).

        Attributes
        -----------

        name : str
            The name of the layer, cannot be longer than 255
        mask : np.ndarray
            The pixel mask applied to the layer
        blend_mode : enum.BlendMode
            The blend mode of the layer, 'Passthrough' is reserved for group layers
        opacity : int
            The layers opacity from 0-255 with 255 being 100%
        width : int
            The width of the layer ranging up to 30,000 for PSD and 300,000 for PSB,
            this does not have to match the files width
        height : int
            The height of the layer ranging up to 30,000 for PSD and 300,000 for PSB,
            this does not have to match the files height
        center_x : float
            The center of the layer in regards to the canvas, a layer at center_x = 0 is
            perfectly centered around the document
        center_y : float
            The center of the layer in regards to the canvas, a layer at center_y = 0 is
            perfectly centered around the document
        is_locked: bool
            The locked state of the layer, this locks all pixel channels
        is_visible: bool
            Whether the layer is visible
    

	)pbdoc";

    layer.def_property("name", &Class::name, &Class::name);
    layer.def_property("mask", [](Class& self)
        {
            std::vector<T> data = self.get_mask_data();
            if (data.empty())
            {
                return py::array_t<T>();
            }
			return to_py_array(std::move(data), self.width(), self.height());
        }, [](Class& self, py::array_t<T> data)
        {
            auto view = from_py_array(tag::view{}, data, self.width(), self.height());
			self.set_mask_data(view);
        });
		
    layer.def_property("blend_mode", &Class::blendmode, &Class::blendmode);
    layer.def_property("is_visible", &Class::visible, &Class::visible);
    layer.def_property("opacity", [](Class& self)
        {
            return self.opacity();
        }, [](Class& self, float opacity)
        {
            self.opacity(opacity);
        });
    layer.def_property("width", &Class::width, &Class::width);
    layer.def_property("height", &Class::height, &Class::height);
    layer.def_property("center_x", &Class::center_x, &Class::center_x);
    layer.def_property("center_y", &Class::center_y, &Class::center_y);
    layer.def_property("is_locked", &Class::locked, &Class::locked);
    layer.def_property("is_visible", &Class::visible, &Class::visible);
	layer.def("has_mask", &Class::has_mask, R"pbdoc(

        Check whether the layer has a mask channel associated with it.

	)pbdoc");
}