#include "LayeredFile/LayerTypes/ImageLayer.h"
#include "LayeredFile/LayerTypes/Layer.h"
#include "Util/Enum.h"
#include "PyUtil/ImageConversion.h"
#include "Implementation/ImageLayer.h"
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
void declareImageLayer(py::module& m, const std::string& extension) {
    using Class = ImageLayer<T>;
    std::string className = "ImageLayer" + extension;
    py::class_<Class, Layer<T>, std::shared_ptr<Class>> imageLayer(m, className.c_str(), py::dynamic_attr(), py::buffer_protocol());

    imageLayer.doc() = R"pbdoc(
        
        This class defines a single image layer in a LayeredFile. There must be at least one of these
        in any given file for it to be valid
    
        Attributes
        -----------

        image_data : dict[int, numpy.ndarray]
            Read-only property: A dictionary of the image data mapped by int.
            Accessing this will load all the image data into memory so use it sparingly and 
            instead try using the num_channels or channels properties.
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

    // Image extraction
    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    imageLayer.def("get_channel_by_id", [](Class& self, const Enum::ChannelID id, const bool do_copy = true)
        {
            std::vector<T> data = self.getChannel(id, do_copy);
            return to_py_array(std::move(data), self.m_Width, self.m_Height);
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
            return to_py_array(std::move(data), self.m_Width, self.m_Height);
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
            return to_py_array(std::move(data), self.m_Width, self.m_Height);
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
            return to_py_array(std::move(data), self.m_Width, self.m_Height);
        }, py::arg("key"), R"pbdoc(
        
	)pbdoc");


    imageLayer.def("__setitem__", [](Class& self, const int key, py::array_t<T>& value)
        {
            auto view = from_py_array(tag::view{}, value, self.m_Width, self.m_Height);
			self.setChannel(static_cast<int16_t>(key), view);
        }, py::arg("key"), py::arg("value"), R"pbdoc(

        Set/replace the channel for a layer at the provided index. 
                
        :param key: The ID or index of the channel
        :type key: :class:`psapi.enum.ChannelID` | int

        :param value: The channel data with dimensions (height, width)
        :type value: np.ndarray
        
	)pbdoc");

    imageLayer.def("__setitem__", [](Class& self, const Enum::ChannelID key, py::array_t<const T>& value)
        {
			auto view = from_py_array(tag::view{}, value, self.m_Width, self.m_Height);
			self.setChannel(key, view);
        }, py::arg("key"), py::arg("value"), R"pbdoc(

	)pbdoc");


    imageLayer.def("set_channel_by_index", [](Class& self, const int key, py::array_t<const T>& value)
        {
			auto view = from_py_array(tag::view{}, value, self.m_Width, self.m_Height);
			self.setChannel(static_cast<int16_t>(key), view);
        }, py::arg("key"), py::arg("value"), R"pbdoc(

        Set/replace the channel for a layer at the provided index. 
                
        :param key: The index of the channel
        :type key: :class: int
        :param value: The channel data with dimensions (height, width)
        :type value: np.ndarray

	)pbdoc");

    imageLayer.def("set_channel_by_id", [](Class& self, const Enum::ChannelID key, py::array_t<const T>& value)
        {
			auto view = from_py_array(tag::view{}, value, self.m_Width, self.m_Height);
			self.setChannel(key, view);
        }, py::arg("key"), py::arg("value"), R"pbdoc(

        Set/replace the channel for a layer at the provided index. 
                
        :param key: The index of the channel
        :type key: :class:`psapi.enum.ChannelID`
        :param value: The channel data with dimensions (height, width)
        :type value: np.ndarray

	)pbdoc");

    imageLayer.def("get_image_data", [](Class& self, const bool do_copy)
        {
            auto data = self.getImageData(do_copy);
            std::unordered_map<int, py::array_t<T>> outData;

            constexpr auto mask_id = Enum::ChannelIDInfo{ Enum::ChannelID::UserSuppliedLayerMask, -2 };
            for (auto& [key, value] : data)
            {
                // Mask channels may have a different resolution compared to the actual layer so we must account for this while parsing.
                if (key == mask_id)
                {
                    if (!self.m_LayerMask)
                    {
                        throw py::value_error("Internal error: Encountered mask channel but layer does not have mask");
                    }
                    auto& mask = self.m_LayerMask.value();
                    auto width = mask.maskData->getWidth();
                    auto height = mask.maskData->getHeight();

                    outData[key.index] = to_py_array(std::move(value), width, height);
                }
                else
                {
                    outData[key.index] = to_py_array(std::move(value), self.m_Width, self.m_Height);
                }
            }
            return outData;
        }, py::arg("do_copy") = true, R"pbdoc(

        Extract all the channels of the ImageLayer into an unordered_map. The channels may have differing sizes 
        as photoshop optimizes mask channels differently than the pixel data
                
        :param do_copy: Defaults to true, Whether to copy the data
        :type do_copy: bool

        :return: The extracted image data
        :rtype: dict[psapi.util.ChannelIDInfo, numpy.ndarray]

	)pbdoc");


    imageLayer.def("set_image_data", py::overload_cast<Class&, std::unordered_map<Enum::ChannelID, py::array_t<T>>&, const Enum::Compression>(&setImageDataFromIDMapping<T>),
        py::arg("image_data"),
        py::arg("compression") = Enum::Compression::ZipPrediction,
        R"pbdoc(

        Replace an image layers' data from image data passed as dict with psapi.enum.ChannelID as key. This function 
        expects all channels to have the same size as the layers width and height similar to the constructor. If 
        you wish to resize and then replace please modify both the layers width and height first. After which you
        can replace it 

        :param image_data: 
            The image data as a dictionary with channel IDs as enums. E.g. for a RGB image layer 

            .. code-block:: python

                data = {
                    psapi.enum.ChannelID.red : numpy.ndarray,
                    psapi.enum.ChannelID.green : numpy.ndarray,
                    psapi.enum.ChannelID.blue : numpy.ndarray
                }

        :type image_data: dict[numpy.ndarray]

        :param compression: The compression to apply to all the channels of the layer, including mask channels. Defaults to ZipPrediction
        :type compression: psapi.enum.Compression

        :raises:
            ValueError: if the channel size is not the same as width * height

	)pbdoc");

    imageLayer.def("set_image_data", py::overload_cast<Class&, std::unordered_map<int, py::array_t<T>>&, const Enum::Compression>(&setImageDataFromIntMapping<T>),
        py::arg("image_data"),
        py::arg("compression") = Enum::Compression::ZipPrediction,
        R"pbdoc(

        Replace an image layers' data from image data passed as dict with int as key. This function 
        expects all channels to have the same size as the layers width and height similar to the constructor. If 
        you wish to resize and then replace please modify both the layers width and height first. After which you
        can replace it 

        :param image_data: 
            The image data as a dictionary with channel IDs as enums. E.g. for a RGB image layer 

            .. code-block:: python

                data = {
                    psapi.enum.ChannelID.red : numpy.ndarray,
                    psapi.enum.ChannelID.green : numpy.ndarray,
                    psapi.enum.ChannelID.blue : numpy.ndarray
                }

        :type image_data: dict[numpy.ndarray]

        :param compression: The compression to apply to all the channels of the layer, including mask channels. Defaults to ZipPrediction
        :type compression: psapi.enum.Compression

        :raises:
            ValueError: if the channel size is not the same as width * height

	)pbdoc");

    imageLayer.def("set_image_data", py::overload_cast<Class&, py::array_t<T>&, const Enum::Compression>(&setImageDataFromNpArray<T>),
        py::arg("image_data"),
        py::arg("compression") = Enum::Compression::ZipPrediction,
        R"pbdoc(

        Replace an image layers' data from image data passed as numpy array. This function 
        expects all channels to have the same size as the layers width and height similar to the constructor. If 
        you wish to resize and then replace please modify both the layers width and height first. After which you
        can replace it 

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

        :param compression: The compression to apply to all the channels of the layer, including mask channels. Defaults to ZipPrediction
        :type compression: psapi.enum.Compression

        :raises:
            ValueError: if the channel size is not the same as width * height

	)pbdoc");

    imageLayer.def("set_compression", &Class::setCompression, py::arg("compression"), R"pbdoc(

        Change the compression codec of all the image channels.
                
        :param compression: The compression codec
        :type compression: :class:`psapi.enum.Compression`

	)pbdoc");

    imageLayer.def_property_readonly("image_data", [](Class& self)
        {
			auto data = self.getImageData(true);
			std::unordered_map<int, py::array_t<T>> outData;

			constexpr auto mask_id = Enum::ChannelIDInfo{ Enum::ChannelID::UserSuppliedLayerMask, -2 };
			for (auto& [key, value] : data)
			{
				// Mask channels may have a different resolution compared to the actual layer so we must account for this while parsing.
				if (key == mask_id)
				{
					if (!self.m_LayerMask)
					{
						throw py::value_error("Internal error: Encountered mask channel but layer does not have mask");
					}
					auto& mask = self.m_LayerMask.value();
					auto width = mask.maskData->getWidth();
					auto height = mask.maskData->getHeight();

					outData[key.index] = to_py_array(std::move(value), width, height);
				}
				else
				{
					outData[key.index] = to_py_array(std::move(value), self.m_Width, self.m_Height);
				}
			}
			return outData;
        }, R"pbdoc(

	)pbdoc");

    imageLayer.def_property_readonly("num_channels", [](Class& self)
        {
            return self.m_ImageData.size();
		});

    imageLayer.def_property_readonly("channels", [](Class& self)
        {
            std::vector<int16_t> indices;
            for (const auto& [key, _] : self.m_ImageData)
            {
                indices.push_back(key.index);
            }
            return indices;
        });
}