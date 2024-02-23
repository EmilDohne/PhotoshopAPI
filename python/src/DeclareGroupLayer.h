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
    py::class_<Class, Layer<T>, std::shared_ptr<Class>> groupLayer(m, className.c_str(), py::dynamic_attr(), R"pbdoc(
        
        Attributes
        -----------
        
        layers : list[psapi.Layer_*]
            The layers under the group, may be empty. These are polymorphic so it may be a group layer, an image layer etc.
            Retrieving them will cast them to their appropriate type
        is_collapsed : bool
            Whether or not the group is collapsed or not
        name : str
            The name of the layer, cannot be longer than 255
        layer_mask : psapi.LayerMask_*
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
    )pbdoc");

    groupLayer.def(py::init(&createGroupLayer<T>), R"pbdoc(
        Construct a group layer instance

        :param layer_name: The name of the group, its length must not exceed 255
        :type layer_name: str

        :param layer_mask: 
            Optional layer mask, must have the same dimensions as height * width but can be a 1- or 2-dimensional array with row-major ordering (for a numpy
            2D array this would mean with a shape of (height, width)
        :type layer_mask: numpy.ndarray

        :param width: 
            Optional, width of the layer, does not have to be the same size as the document, limited to 30,000 for PSD files and 300,000 for PSB files.
            For group layers this is only relevant for the layer mask and can be left out otherwise
        :type width: int

        :param height: 
            Optional, height of the layer, does not have to be the same size as the document, limited to 30,000 for PSD files and 300,000 for PSB files.
            For group layers this is only relevant for the layer mask and can be left out otherwise
        :type height: int

        :param blend_mode: Optional, the blend mode of the layer, 'Passthrough' is the default for groups.
        :type blend_mode: psapi.enum.BlendMode

        :param pos_x: 
            Optional, the relative offset of the layer to the center of the document, 0 indicates the layer is centered.
            For group layers this is only relevant for the layer mask and can be left out otherwise
        :type pos_x: int

        :param pos_y: 
            Optional, the relative offset of the layer to the center of the document, 0 indicates the layer is centered.
            For group layers this is only relevant for the layer mask and can be left out otherwise
        :type pos_y: int

        :param opacity: The opacity of the layer from 0-255 where 0 is 0% and 255 is 100%. Defaults to 255
        :type opacity: int

        :param compression: The compression to apply to all the channels of the layer, including mask channels
        :type compression: psapi.enum.Compression

        :param color_mode: The color mode of the Layer, this must be identical to the color mode of the document. Defaults to RGB
        :type color_mode: psapi.enum.ColorMode

        :param is_collapsed: Whether or not the group is collapsed (closed)
        :type is_collapsed: bool

        :raises:
            ValueError: if length of layer name is greater than 255

            ValueError: if size of layer mask is not width*height

            ValueError: if width of layer is negative

            ValueError: if height of layer is negative

            ValueError: if opacity is not between 0-255
    )pbdoc",
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

    groupLayer.def("add_layer", &Class::addLayer, R"pbdoc(
        Add the specified layer to the group

        :param layered_file: The top level LayeredFile instance, required to ensure a layer doesnt get added twice
        :type layered_file: psapi.LayeredFile_8bit | psapi.LayeredFile_16bit | psapi.LayeredFile_32bit

        :param layer: the layer instance to insert under the group
    )pbdoc", py::arg("layered_file"), py::arg("layer"));

    groupLayer.def("remove_layer", py::overload_cast<const int>(&Class::removeLayer), R"pbdoc(
        Remove the specified layer from the group, raises a warning if the index isnt valid

        :param index: The index of the layer to be removed
        :type index: int
    )pbdoc", py::arg("index"));
    groupLayer.def("remove_layer", py::overload_cast<std::shared_ptr<Layer<T>>&>(&Class::removeLayer), R"pbdoc(
        Remove the specified layer from the group, raises a warning if the layer is not under the group

        :param layer: The layer instance to remove
        :type layer: LayerType
    )pbdoc", py::arg("layer"));
    groupLayer.def("remove_layer", py::overload_cast<const std::string>(&Class::removeLayer),
        R"pbdoc(
        Remove the specified layer from the group, raises a warning if the layer is not under the group

        :param layer_name: The name of the layer to remove
        :type layer: str
    )pbdoc", py::arg("layer_name"));

    // Implement dict-like indexing for group layers with the [] syntax
    groupLayer.def("__getitem__", [](const Class& self, const std::string name)
        {
            for (auto& layer : self.m_Layers)
            {
                // Check if the layers are the same
                if (layer->m_LayerName == name)
                {   
                    // pybind automatically downcasts this to the appropriate type so no action is needed
                    return layer;
                }
            }
            throw py::key_error("Unable to find layer '" + name + "' in the Group");
        }, R"pbdoc(
		Get the specified layer from the group using dict-like indexing. This may be chained as deep as the layer hierarchy goes

		.. code-block:: python

			group_layer: GroupLayer_8bit = # Our group layer instance
			nested_img_layer = group_layer["NestedGroup"]["Image"]

		:param name: The name of the layer to search for
		:type name: str

		:raises:
			KeyError: If the requested layer is not found
            
		:return: The requested layer instance

		)pbdoc", py::arg("name"));
}