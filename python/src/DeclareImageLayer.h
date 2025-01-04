#include "LayeredFile/LayerTypes/ImageLayer.h"
#include "LayeredFile/LayerTypes/Layer.h"
#include "Util/Enum.h"
#include "PyUtil/ImageConversion.h"
#include "Implementation/ImageDataLayerType.h"
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
void declare_image_layer(py::module& m, const std::string& extension) {
    using Class = ImageLayer<T>;
    std::string className = "ImageLayer" + extension;
    py::class_<Class, Layer<T>, std::shared_ptr<Class>> imageLayer(m, className.c_str(), py::dynamic_attr(), py::buffer_protocol());

    imageLayer.doc() = R"pbdoc(
        
        This class defines a single image layer in a LayeredFile. There must be at least one of these
        in any given file for it to be valid
    
        Attributes
        -----------

         image_data : dict[int, numpy.ndarray]
            Property: A dictionary of the image data mapped by an int where the channel mapping
            is e.g. [R: 0, G: 1, B: 2]. Accessing this property will decompress and load the image
            data into memory therefore incurring a performance and memory penalty. If you only wish
            to get a list of all the channels use the `num_channels` or `channels` properties instead.

            All channels are the same size except for the mask channel (-2) which may have any size.
        num_channels: int
            Read-only property: The number of channels held by image_data

        channels: list[int]
            Read-only property: The channel indices held by this image layer. 
            Unlike accessing image_data this does not extract the image data and is therefore
            near-zero cost.
        name : str
            The name of the layer, cannot be longer than 255
        layer_mask : LayerMask_*bit
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

    // Constructors
    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------

    imageLayer.def(py::init(&createImageLayerFromNpArray<T>),
        py::arg("image_data"),
        py::arg("layer_name"),
        py::arg("layer_mask") = py::none(),
        py::arg("width") = 0,
        py::arg("height") = 0,
        py::arg("blend_mode") = Enum::BlendMode::Normal,
        py::arg("pos_x") = 0,
        py::arg("pos_y") = 0,
        py::arg("opacity") = 255,
        py::arg("compression") = Enum::Compression::ZipPrediction,
        py::arg("color_mode") = Enum::ColorMode::RGB,
        py::arg("is_visible") = true,
        py::arg("is_locked") = false, R"pbdoc(

        Construct an image layer from image data passed as numpy.ndarray

        :param image_data: 
            The image data as 2- or 3-Dimensional numpy array where the first dimension is the number of channels.
    
            If its a 2D ndarray the second dimension must hold the image data in row-major order with the size being height*width. 
            An example could be the following shape: (3, 1024) for an RGB layer that is 32*32px. 

            If its a 3D ndarray the second and third dimension hold height and width respectively.
            An example could be the following shape: (3, 32, 32) for the same RGB layer

            We also support adding alpha channels this way, those are always stored as the last channel and are optional. E.g. for RGB
            there could be a ndarray like this (4, 32, 32) and it would automatically identify the last channel as alpha. For the individual
            color modes there is always a set of required channels such as R, G and B for RGB or C, M, Y, K for CMYK with the optional alpha
            that can be appended to the end.

            The size **must** be the same as the width and height parameter
        :type image_data: numpy.ndarray

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

            ValueError: if the channel size is not the same as width * height

	)pbdoc");

    imageLayer.def(py::init(&createImageLayerFromIntMapping<T>),
        py::arg("image_data"),
        py::arg("layer_name"),
        py::arg("layer_mask") = py::none(),
        py::arg("width") = 0,
        py::arg("height") = 0,
        py::arg("blend_mode") = Enum::BlendMode::Normal,
        py::arg("pos_x") = 0,
        py::arg("pos_y") = 0,
        py::arg("opacity") = 255,
        py::arg("compression") = Enum::Compression::ZipPrediction,
        py::arg("color_mode") = Enum::ColorMode::RGB,
        py::arg("is_visible") = true,
        py::arg("is_locked") = false, R"pbdoc(

        Construct an image layer from image data passed as dict with integers as key

        :param image_data: 
            The image data as a dictionary with channel indices as integers. E.g. for a RGB image layer 
            
            .. code-block:: python

                data = {
                    0 : numpy.ndarray,
                    1 : numpy.ndarray,
                    2 : numpy.ndarray
                }

        :type image_data: dict[numpy.ndarray]

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

            ValueError: if the channel size is not the same as width * height

	)pbdoc");

    imageLayer.def(py::init(&createImageLayerFromIDMapping<T>),
        py::arg("image_data"),
        py::arg("layer_name"),
        py::arg("layer_mask") = py::none(),
        py::arg("width") = 0,
        py::arg("height") = 0,
        py::arg("blend_mode") = Enum::BlendMode::Normal,
        py::arg("pos_x") = 0,
        py::arg("pos_y") = 0,
        py::arg("opacity") = 255,
        py::arg("compression") = Enum::Compression::ZipPrediction,
        py::arg("color_mode") = Enum::ColorMode::RGB,
        py::arg("is_visible") = true,
        py::arg("is_locked") = false, R"pbdoc(

        Construct an image layer from image data passed as dict with psapi.enum.ChannelID as key

        :param image_data: 
            The image data as a dictionary with channel IDs as enums. E.g. for a RGB image layer 

            .. code-block:: python

                data = {
                    psapi.enum.ChannelID.red : numpy.ndarray,
                    psapi.enum.ChannelID.green : numpy.ndarray,
                    psapi.enum.ChannelID.blue : numpy.ndarray
                }

        :type image_data: dict[numpy.ndarray]

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

            ValueError: if the channel size is not the same as width * height

	)pbdoc");

}