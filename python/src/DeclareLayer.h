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

    layer.doc() = R"pbdoc(

        Base type that all layers inherit from, this class should not be instantiated
        and instead the derivatives such as :class:`psapi.GroupLayer_8bit` or :class:`psapi.ImageLayer_8bit`
        should be used (with the appropriate bit depth).

        Attributes
        -----------

        name : str
            The name of the layer, cannot be longer than 255
        layer_mask : np.ndarray
            The pixel mask applied to the layer, read only
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
    

	)pbdoc";

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
            return py::array_t<T>(0, nullptr);
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
        }, R"pbdoc(

        Get the pixel mask data associated with the layer (if it exists), if it doesnt
        a warning gets raised and a null-size numpy.ndarray is returned.

        The size of the mask is not necessarily the same as the layer

        :param do_copy: Whether or not to copy the image data on extraction, if False the mask channel is freed
        :type do_copy: bool

        :return: The extracted channel with dimensions (mask_height, mask_width)
        :rtype: numpy.ndarray        

	)pbdoc");
}