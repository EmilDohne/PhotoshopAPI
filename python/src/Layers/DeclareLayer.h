#include "LayeredFile/LayerTypes/Layer.h"
#include "LayeredFile/LayerTypes/MaskDataMixin.h"
#include "Macros.h"
#include "PyUtil/ImageConversion.h"
#include "Mixins/DeclareMaskMixin.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>
#include <pybind11/iostream.h>

#include <memory>
#include <optional>
#include <span>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


// Generate a LayeredFile python class from our struct adjusting some
// of the methods 
template <typename T>
void declare_layer(py::module& m, const std::string& extension) {
    using Class = Layer<T>;
    // Designate shared_ptr as the holder type so pybind11 doesnt cast to unique_ptr giving us a heap corruption error
    using PyClass = py::class_<Class, std::shared_ptr<Class>>;
    std::string className = "Layer" + extension;

    PyClass layer(m, className.c_str(), py::dynamic_attr(), py::buffer_protocol());
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
        opacity : float
            The layers opacity from 0.0 - 1.0
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
        mask: np.ndarray
            The layers' mask channel, may be empty
        mask_disabled: bool
            Whether the mask is disabled. Ignored if no mask is present
        mask_relative_to_layer: bool
            Whether the masks position is relative to the layer. Ignored if no mask is present
        mask_default_color: int
            The masks' default color outside of the masks bounding box from 0-255. Ignored if no mask is present
        mask_density: int
            Optional mask density from 0-255, this is equivalent to layers' opacity. Ignored if no mask is present
        mask_feather: float
            Optional mask feather. Ignored if no mask is present
        mask_position: psapi.geometry.Point2D
            The masks' canvas coordinates, these represent the center of the mask in terms of the canvas (file). Ignored if no mask is present
        mask_width: int
            The masks' width, this does not have to correspond with the layers' width
        mask_height: int
            The masks' height, this does not have to correspond with the layers' height

	)pbdoc";

    bind_mask_mixin<T, Class, PyClass>(layer);

    layer.def_property("name", &Class::name, &Class::name);		
    layer.def_property("blend_mode", [](const Class& self) { return self.blendmode(); }, [](Class& self, Enum::BlendMode blendmode) { self.blendmode(blendmode); });
    layer.def_property("opacity", [](const Class& self) { return self.opacity(); }, [](Class& self, float opacity) { self.opacity(opacity); });
    layer.def_property("width", [](const Class& self) { return self.width(); }, [](Class& self, uint32_t width) { self.width(width); });
    layer.def_property("height", [](const Class& self) { return self.height(); }, [](Class& self, uint32_t height) { self.height(height); });
    layer.def_property("center_x", [](const Class& self) { return self.center_x(); }, [](Class& self, float center_x) { self.center_x(center_x); });
    layer.def_property("center_y", [](const Class& self) { return self.center_y(); }, [](Class& self, float center_y) { self.center_y(center_y); });
    layer.def_property("is_locked", [](const Class& self) { return self.locked(); }, [](Class& self, bool locked) { self.locked(locked); });
    layer.def_property("is_visible", [](const Class& self) { return self.visible(); }, [](Class& self, bool visible) { self.visible(visible); });
}