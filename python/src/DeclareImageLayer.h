#include "LayeredFile/LayerTypes/ImageLayer.h"
#include "LayeredFile/LayerTypes/Layer.h"
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

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


// Generate CPP image data based on the provided numpy array which can either come as 2d or 3d array.
template <typename T>
std::unordered_map<Enum::ChannelID, std::vector<T>> generateImageData(py::array_t<T>& image_data, int width, int height, const Enum::ColorMode color_mode)
{
    // Generate an unordered map with implicit bindings for the channels using a 2d or 3d numpy array
    std::unordered_map<Enum::ChannelID, std::vector<T>> img_data_cpp;

    // This will e.g. be (3, 1024, 1024) for a 3 channel 1024x1024 image
    std::vector<size_t> shape;
    for (size_t i = 0; i < image_data.ndim(); ++i)
    {
        shape.push_back(image_data.shape(i));
    }
    if (shape.size() != 2 && shape.size() != 3)
    {
        throw py::value_error("image_data parameter must have either 2 or 3 dimensions, not " + std::to_string(shape.size()));
    }

    

    // Extract the size of one channel and compare it against the size of the data
    size_t channelSize = 0;
    if (shape.size() == 2)
    {
        channelSize = shape[1]; // Width * Height for a 2D image
        if (channelSize != static_cast<size_t>(width) * height)
        {
            throw py::value_error("image_data parameter is expected to be of shape (channels, height * width) or (channels, height, width)" \
" but the provided 2nd dimension does not match the provided width * height, got: " + std::to_string(channelSize) + " but instead expected: " \
+ std::to_string(static_cast<size_t>(width) * height) + ".\nThis is likely due to having an incorrectly shaped array or providing an incorrect width or height");
        }
    }
    else if (shape.size() == 3)
    {
        channelSize = shape[1] * shape[2]; // Width * Height for a 3D image
        if (channelSize != static_cast<size_t>(width) * height)
        {
            throw py::value_error("image_data parameter is expected to be of shape (channels, height * width) or (channels, height, width)" \
" but the provided 2nd and 3rd dimension do not match the provided width * height, got: " + std::to_string(channelSize) + " but instead expected : " \
+ std::to_string(static_cast<size_t>(width) * height) + ".\nThis is likely due to having an incorrectly shaped array or providing an incorrect width or height");
        }
    }

    // Force the array to be c-contiguous if it isnt using pybind11
    if (py::detail::npy_api::constants::NPY_ARRAY_C_CONTIGUOUS_ != (image_data.flags() & py::detail::npy_api::constants::NPY_ARRAY_C_CONTIGUOUS_))
    {
        PSAPI_LOG_WARNING("ImageLayer", "Provided image_data parameter was detected to not be c-style contiguous, forcing this conversion in-place");
        image_data = image_data.template cast<py::array_t<T, py::array::c_style | py::array::forcecast>>();
    }

    // For RGB we have two options, either there is an alpha channel with the RGB channels or not.
    if (color_mode == Enum::ColorMode::RGB)
    {
        if (shape[0] != 3 && shape[0] != 4)
        {
            throw py::value_error("Passed array must have either 3 or 4 channels, not " + std::to_string(shape[0]));
        }
        std::vector<Enum::ChannelID> rgbChannelIDs = { Enum::ChannelID::Red, Enum::ChannelID::Green, Enum::ChannelID::Blue, Enum::ChannelID::Alpha };
        for (size_t i = 0; i < shape[0]; ++i)
        {
            std::vector<T> channelData(channelSize);
            const T* startPtr = image_data.data() + i * channelSize;
            std::memcpy(reinterpret_cast<uint8_t*>(channelData.data()), reinterpret_cast<const uint8_t*>(startPtr), channelSize * sizeof(T));
            img_data_cpp[rgbChannelIDs[i]] = channelData;
        }
        return img_data_cpp;
    }

    // We add preliminary support for CMYK but it is not fully supported yet!
    if (color_mode == Enum::ColorMode::CMYK)
    {
        if (shape[0] != 4 && shape[0] != 5)
        {
            throw py::value_error("Passed array must have either 4 or 5 channels, not " + std::to_string(shape[0]));
        }
        std::vector<Enum::ChannelID> cmykChannelIds = { Enum::ChannelID::Cyan, Enum::ChannelID::Magenta, Enum::ChannelID::Yellow, Enum::ChannelID::Black, Enum::ChannelID::Alpha };
        for (size_t i = 0; i < shape[0]; ++i)
        {
            std::vector<T> channelData(channelSize);
            const T* startPtr = image_data.data() + i * channelSize;
            std::memcpy(reinterpret_cast<uint8_t*>(channelData.data()), reinterpret_cast<const uint8_t*>(startPtr), channelSize * sizeof(T));

            img_data_cpp[cmykChannelIds[i]] = channelData;
        }
        return img_data_cpp;
    }

    // We add preliminary support for greyscale but it is not fully supported yet!
    if (color_mode == Enum::ColorMode::Grayscale)
    {
        if (shape[0] != 1 && shape[0] != 2)
        {
            throw py::value_error("Passed array must have either 1 or 2 channels, not " + std::to_string(shape[0]));
        }
        std::vector<Enum::ChannelID> greyChannelIDs = { Enum::ChannelID::Gray, Enum::ChannelID::Alpha};
        for (size_t i = 0; i < shape[0]; ++i)
        {
            std::vector<T> channelData(channelSize);
            const T* startPtr = image_data.data() + i * channelSize;
            std::memcpy(reinterpret_cast<uint8_t*>(channelData.data()), reinterpret_cast<const uint8_t*>(startPtr), channelSize * sizeof(T));

            img_data_cpp[greyChannelIDs[i]] = channelData;
        }
        return img_data_cpp;
    }

    throw py::value_error("Unsupported color mode encountered when trying to parse numpy array to image dict");
}


// Create an alternative constructor which inlines the Layer<T>::Params since the more pythonic version would be to have kwargs rather 
// than a separate structure as well as creating an interface for numpy.
template <typename T>
std::shared_ptr<ImageLayer<T>> createImageLayerFromNpArray(
    py::array_t<T>& image_data,
    const std::string& layer_name,
    const std::optional<py::array_t<T>> layer_mask,
    int width,  // This is only relevant if a layer mask is set
    int height, // This is only relevant if a layer mask is set
    const Enum::BlendMode blend_mode,
    int pos_x, // This is only relevant if a layer mask is set
    int pos_y, // This is only relevant if a layer mask is set
    int opacity,
    const Enum::Compression compression,
    const Enum::ColorMode color_mode
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
    
    // Generate an unordered dict from the image data trying to automatically decode channels into their corresponding
    // channel mappings 
    auto img_data_cpp = generateImageData(image_data, width, height, color_mode);

    params.layerName = layer_name;
    params.blendMode = blend_mode;
    params.posX = pos_x;
    params.posY = pos_y;
    params.width = width;
    params.height = height;
    params.opacity = opacity;
    params.compression = compression;
    params.colorMode = color_mode;
    return std::make_shared<ImageLayer<T>>(std::move(img_data_cpp), params);
}


// Create an alternative constructor which inlines the Layer<T>::Params since the more pythonic version would be to have kwargs rather 
// than a separate structure as well as creating an interface for numpy.
template <typename T>
std::shared_ptr<ImageLayer<T>> createImageLayerFromIDMapping(
    std::unordered_map<Enum::ChannelID, py::array_t<T>>& image_data,
    const std::string& layer_name,
    const std::optional<py::array_t<T>> layer_mask,
    int width,  // This is only relevant if a layer mask is set
    int height, // This is only relevant if a layer mask is set
    const Enum::BlendMode blend_mode,
    int pos_x, // This is only relevant if a layer mask is set
    int pos_y, // This is only relevant if a layer mask is set
    int opacity,
    const Enum::Compression compression,
    const Enum::ColorMode color_mode
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
    std::unordered_map<Enum::ChannelID, std::vector<T>> img_data_cpp;
    // Convert our image data to c++ vector data, the constructor checks for the right amount of channels
    for (auto& [key, value] : image_data)
    {
        if (value.size() != static_cast<uint64_t>(width) * height)
        {
            throw py::value_error("Channel '" + Enum::channelIDToString(key) + "' must have the same size as the layer itself (width * height)");
        }
        img_data_cpp[key] = std::vector<T>(value.data(), value.data() + value.size());
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
    return std::make_shared<ImageLayer<T>>(std::move(img_data_cpp), params);
}


// Create an alternative constructor which inlines the Layer<T>::Params since the more pythonic version would be to have kwargs rather 
// than a separate structure as well as creating an interface for numpy.
template <typename T>
std::shared_ptr<ImageLayer<T>> createImageLayerFromIntMapping(
    std::unordered_map<int, py::array_t<T>>& image_data,
    const std::string& layer_name,
    const std::optional<py::array_t<T>> layer_mask,
    int width,  // This is only relevant if a layer mask is set
    int height, // This is only relevant if a layer mask is set
    const Enum::BlendMode blend_mode,
    int pos_x, // This is only relevant if a layer mask is set
    int pos_y, // This is only relevant if a layer mask is set
    int opacity,
    const Enum::Compression compression,
    const Enum::ColorMode color_mode
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
    std::unordered_map<int16_t, std::vector<T>> img_data_cpp;
    // Convert our image data to c++ vector data, the constructor checks for the right amount of channels
    for (auto& [key, value] : image_data)
    {
        if (value.size() != static_cast<uint64_t>(width) * height)
        {
            throw py::value_error("Channel '" + std::to_string(key) + "' must have the same size as the layer itself (width * height)");
        }
        img_data_cpp[static_cast<int16_t>(key)] = std::vector<T>(value.data(), value.data() + value.size());
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
    return std::make_shared<ImageLayer<T>>(std::move(img_data_cpp), params);
}


// Generate a LayeredFile python class from our struct adjusting some
// of the methods 
template <typename T>
void declareImageLayer(py::module& m, const std::string& extension) {
	using Class = ImageLayer<T>;
	std::string className = "ImageLayer" + extension;
	py::class_<Class, Layer<T>, std::shared_ptr<Class>> imageLayer(m, className.c_str(), py::dynamic_attr(), py::buffer_protocol());

    imageLayer.doc() = R"pbdoc(
        
        This class defines a single image layer in a LayeredFile. There must be at least one of these
        in any given file for it to be valid
    
        Attributes
        -----------

        image_data : dict[numpy.ndarray]
            A dictionary of the image data mapped by :class:`psapi.util.ChannelIDInfo`
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
        py::arg("color_mode") = Enum::ColorMode::RGB, R"pbdoc(

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
        py::arg("color_mode") = Enum::ColorMode::RGB, R"pbdoc(

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
        py::arg("color_mode") = Enum::ColorMode::RGB, R"pbdoc(

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

        :raises:
            ValueError: if length of layer name is greater than 255

            ValueError: if size of layer mask is not width*height

            ValueError: if width of layer is negative

            ValueError: if height of layer is negative

            ValueError: if opacity is not between 0-255

            ValueError: if the channel size is not the same as width * height

	)pbdoc");

    // Image extraction
    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    imageLayer.def("get_channel_by_id", [](Class& self, const Enum::ChannelID id, const bool do_copy = true)
        {
            std::vector<T> data = self.getChannel(id, do_copy);

            // Get pointer to copied data and size
            T* ptr = data.data();
            std::vector<size_t> shape = { self.m_Height, self.m_Width };

            return py::array_t<T>(shape, ptr);
        }, py::arg("id"), py::arg("do_copy") = true, R"pbdoc(

        Extract a specified channel from the layer given its channel ID.
                
        :param id: The ID of the channel
        :type id: :class:`psapi.enum.ChannelID`

        :param do_copy: Defaults to true, whether to copy the data on extraction (if false the channel is invalidated)
        :type do_copy: bool            

        :return: The extracted channel
        :rtype: numpy.ndarray

	)pbdoc");

    imageLayer.def("get_channel_by_index", [](Class& self, const int index, const bool do_copy = true)
        {
            std::vector<T> data = self.getChannel(static_cast<int16_t>(index), do_copy);

            // Get pointer to copied data and size
            T* ptr = data.data();
            std::vector<size_t> shape = { self.m_Height, self.m_Width };

            return py::array_t<T>(shape, ptr);
        }, py::arg("index"), py::arg("do_copy") = true, R"pbdoc(

        Extract a specified channel from the layer given its channel index.
                
        :param index: The index of the channel
        :type index: int

        :param do_copy: Defaults to true, whether to copy the data on extraction (if false the channel is invalidated)
        :type do_copy: bool            

        :return: The extracted channel with dimensions (height, width)
        :rtype: numpy.ndarray

	)pbdoc");

    imageLayer.def("__getitem__", [](Class& self, const Enum::ChannelID key)
        {
            std::vector<T> data = self.getChannel(key, true);

            // Get pointer to copied data and size
            T* ptr = data.data();
            std::vector<size_t> shape = { self.m_Height, self.m_Width };

            if (data.size() != self.m_Height * self.m_Width)
            {
                throw py::key_error("Unable to retrieve channel " + Enum::channelIDToString(key));
            }

            return py::array_t<T>(shape, ptr);
        }, py::arg("key"), R"pbdoc(

        Extract a specified channel from the layer given its channel index.
                
        :param key: The ID or index of the channel
        :type key: :class:`psapi.enum.ChannelID` | int

        :return: The extracted channel with dimensions (height, width)
        :rtype: np.ndarray

	)pbdoc");

    imageLayer.def("__getitem__", [](Class& self, const int key)
        {
            std::vector<T> data = self.getChannel(key, true);

            // Get pointer to copied data and size
            T* ptr = data.data();
            std::vector<size_t> shape = { self.m_Height, self.m_Width };

            if (data.size() != self.m_Height * self.m_Width)
            {
                throw py::key_error("Unable to retrieve channel index " + std::to_string(key));
            }
            return py::array_t<T>(shape, ptr);
        }, py::arg("key"), R"pbdoc(

	)pbdoc");

    imageLayer.def("get_image_data", [](Class& self, const bool do_copy)
        {
            auto data = self.getImageData(do_copy);
            std::unordered_map<int, py::array_t<T>> outData;
            for (auto& [key, value] : data)
            {
                auto width = static_cast<size_t>(self.m_Width);
                auto height = static_cast<size_t>(self.m_Height);
                std::vector<size_t> shape = { height, width };
                T* ptr = value.data();
                // Unfortunately I dont think this can be trivially move constructed
                outData[key.index] = py::array_t<T>(shape, ptr);
            }
            return outData;
        }, py::arg("do_copy") = true, R"pbdoc(

        Extract all the channels of the ImageLayer into an unordered_map.
                
        :param do_copy: Defaults to true, Whether to copy the data
        :type do_copy: bool

        :return: The extracted image data
        :rtype: dict[psapi.util.ChannelIDInfo, numpy.ndarray]

	)pbdoc");

    imageLayer.def("set_compression", &Class::setCompression, py::arg("compression"), R"pbdoc(

        Change the compression codec of all the image channels.
                
        :param compression: The compression codec
        :type compression: :class:`psapi.enum.Compression`

	)pbdoc");

    imageLayer.def_property_readonly("image_data", [](Class& self)
        {
            auto data = self.getImageData();
            std::unordered_map<int, py::array_t<T>> outData;
            for (auto& [key, value] : data)
            {
                auto width = static_cast<size_t>(self.m_Width);
                auto height = static_cast<size_t>(self.m_Height);
                std::vector<size_t> shape = { height, width };
                T* ptr = value.data();
                // Unfortunately I dont think this can be trivially move constructed
                outData[key.index] = py::array_t<T>(shape, ptr);
            }
            return outData;
        }, R"pbdoc(

	)pbdoc");
}