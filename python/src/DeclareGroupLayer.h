#include "LayeredFile/LayerTypes/GroupLayer.h"
#include "LayeredFile/LayerTypes/Layer.h"
#include "Implementation/GroupLayer.h"
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

// Generate a LayeredFile python class from our struct adjusting some
// of the methods 
template <typename T>
void declareGroupLayer(py::module& m, const std::string& extension) {
    using Class = GroupLayer<T>;
    std::string className = "GroupLayer" + extension;
    py::class_<Class, Layer<T>, std::shared_ptr<Class>> groupLayer(m, className.c_str(), py::dynamic_attr());

    groupLayer.doc() = R"pbdoc(

	    Attributes
        -----------

        layers : list[psapi.Layer_*bit]
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
        is_locked: bool
            The locked state of the layer, this locks all pixel channels
        is_visible: bool
            Whether the layer is visible

	)pbdoc";

    groupLayer.def(py::init(&createGroupLayer<T>),
        py::arg("layer_name"),
        py::arg("layer_mask") = py::none(),
        py::arg("width") = 0,
        py::arg("height") = 0,
        py::arg("blend_mode") = Enum::BlendMode::Passthrough,
        py::arg("pos_x") = 0,
        py::arg("pos_y") = 0,
        py::arg("opacity") = 255,
        py::arg("compression") = Enum::Compression::ZipPrediction,
        py::arg("color_mode") = Enum::ColorMode::RGB,
        py::arg("is_collapsed") = false,
        py::arg("is_visible") = true,
        py::arg("is_locked") = false, R"pbdoc(

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

            :param is_collapsed: Whether the group is collapsed (closed)
            :type is_collapsed: bool

            :param is_visible: Whether the group is visible
            :type is_visible: bool

            :param is_locked: Whether the group is locked
            :type is_locked: bool

            :raises:
                ValueError: if length of layer name is greater than 255

                ValueError: if size of layer mask is not width*height

                ValueError: if width of layer is negative

                ValueError: if height of layer is negative

                ValueError: if opacity is not between 0-255
		)pbdoc");

    groupLayer.def_property("layers", [](Class& self)
        {
            return self.layers();
        }, [](Class& self, std::vector<std::shared_ptr<Layer<T>>>& layers)
        {
            self.layers(layers);
        });
    groupLayer.def_property("is_collapsed", [](Class& self)
        {
            return self.collapsed();
        }, [](Class& self, bool collapsed)
        {
            self.collapsed(collapsed);
        });

    groupLayer.def("add_layer", &Class::add_layer, py::arg("layered_file"), py::arg("layer"), R"pbdoc(

        Add the specified layer to the group

        :param layered_file: The top level LayeredFile instance, required to ensure a layer doesnt get added twice
        :type layered_file: psapi.LayeredFile_*bit

        :param layer: the layer instance to insert under the group
        :type layer: Layer_*bit
	)pbdoc");

    groupLayer.def("remove_layer", py::overload_cast<const int>(&Class::remove_layer), py::arg("index"), R"pbdoc(

        Remove the specified layer from the group, raises a warning if the index isnt valid

        :param index: The index of the layer to be removed
        :type index: int

	)pbdoc");
    groupLayer.def("remove_layer", py::overload_cast<std::shared_ptr<Layer<T>>&>(&Class::remove_layer), py::arg("layer"), R"pbdoc(

        Remove the specified layer from the group, raises a warning if the layer isnt under the group

        :param layer: The layer to be removed
        :type layer: Layer_*bit

	)pbdoc");
    groupLayer.def("remove_layer", py::overload_cast<const std::string>(&Class::remove_layer), py::arg("layer_name"), R"pbdoc(
        
        Remove the specified layer from the group, raises a warning if the layer isnt under the group

        :param layer_name: The layer to be removed
        :type layer_name: str

	)pbdoc");

    // Implement dict-like indexing for group layers with the [] syntax
    groupLayer.def("__getitem__", [](const Class& self, const std::string value)
        {
            for (auto& layer : self.layers())
            {
                // Check if the layers are the same
                if (layer->name() == value)
                {   
                    // pybind automatically downcasts this to the appropriate type so no action is needed
                    return layer;
                }
            }
            throw py::key_error("Unable to find layer '" + value + "' in the Group");
        }, py::arg("value"), R"pbdoc(

        Get the specified layer from the group using dict-like indexing. This may be chained as deep as the layer hierarchy goes

        .. code-block:: python

            group_layer: GroupLayer_*bit = # Our group layer instance
            nested_img_layer = group_layer["NestedGroup"]["Image"]

        :param value: The name of the layer to search for
        :type value: str

        :raises:
            KeyError: If the requested layer is not found
            
        :return: The requested layer instance

	)pbdoc");
}