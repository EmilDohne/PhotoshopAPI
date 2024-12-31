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
void declare_ImageDataLayerType(py::module& m, const std::string& extension) 
{

    using Class = _ImageDataLayerType<T>;
    std::string className = "_ImageDataLayerType" + extension;
    py::class_<Class, Layer<T>, std::shared_ptr<Class>> _imageDataLayer(m, className.c_str(), py::dynamic_attr(), py::buffer_protocol());

    _imageDataLayer.doc() = R"pbdoc(
        
        This class defines a shared base for all layers dealing with image data (ImageLayer and SmartObjectLayer). This class isn't meant to be instantiated
        directly similar to the Layer class. Provides an interface for storing, retrieving and setting image data. 

        Has the dunder methods `__getitem__` and `__setitem__` mapped to allow access to channels by indexing. So e.g. to get the red channel one can
        access it as such: `layer[0]`. Similarly, setting an item is supported that way `layer[0] = np.ndarray(...)`

        All channels must have the same size with the exception of the mask channel which is independant and may be any other size. So if the layer is e.g.
        1024x1024 pixels it's perfectly valid to have the mask be 256x256. Photoshop does in fact commonly do this to optimize empty space and make the masks'
        bounding box tightly fitting.

        For maximum efficiency it is however recommended to set the whole image data directly as that parallelizes better
    
        Attributes
        -----------

        image_data : dict[int, numpy.ndarray]
            Read-only property: A dictionary of the image data mapped by an int where the channel mapping
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
        
    )pbdoc";

    // Constructors
    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------

    // We don't provide any explicit constructors as the layer isn't to be constructed directly. On the cpp side we do have 
    // ctors that will forward the information though, this is just to make these more opaque

    // Image extraction
    // ---------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------
    _imageDataLayer.def("get_channel_by_id", [](Class& self, const Enum::ChannelID id, const bool do_copy = true)
        {
            std::vector<T> data = self.get_channel(id, do_copy);
            return to_py_array(std::move(data), self.width(), self.height());
        }, py::arg("id"), py::arg("do_copy") = true, R"pbdoc(

        Extract a specified channel from the layer given its channel ID.
                
        :param id: The ID of the channel
        :type id: :class:`psapi.enum.ChannelID`

        :param do_copy: Defaults to true, whether to copy the data on extraction (if false the channel is invalidated)
        :type do_copy: bool            

        :return: The extracted channel
        :rtype: numpy.ndarray

	)pbdoc");

    _imageDataLayer.def("get_channel_by_index", [](Class& self, const int index, const bool do_copy = true)
        {
            std::vector<T> data = self.get_channel(static_cast<int16_t>(index), do_copy);
            return to_py_array(std::move(data), self.width(), self.height());
        }, py::arg("index"), py::arg("do_copy") = true, R"pbdoc(

        Extract a specified channel from the layer given its channel index.
                
        :param index: The index of the channel
        :type index: int

        :param do_copy: Defaults to true, whether to copy the data on extraction (if false the channel is invalidated)
        :type do_copy: bool            

        :return: The extracted channel with dimensions (height, width)
        :rtype: numpy.ndarray

	)pbdoc");

    _imageDataLayer.def("__getitem__", [](Class& self, const Enum::ChannelID key)
        {
            std::vector<T> data = self.get_channel(key, true);
            return to_py_array(std::move(data), self.width(), self.height());
        }, py::arg("key"), R"pbdoc(

        Extract a specified channel from the layer given its channel index.
                
        :param key: The ID or index of the channel
        :type key: :class:`psapi.enum.ChannelID` | int

        :return: The extracted channel with dimensions (height, width)
        :rtype: np.ndarray

	)pbdoc");

    _imageDataLayer.def("__getitem__", [](Class& self, const int key)
        {
            std::vector<T> data = self.get_channel(key, true);
            return to_py_array(std::move(data), self.width(), self.height());
        }, py::arg("key"), R"pbdoc(
        
	)pbdoc");


    _imageDataLayer.def("__setitem__", [](Class& self, const int key, py::array_t<T>& value)
        {
            auto view = from_py_array(tag::view{}, value, self.width(), self.height());
            self.set_channel(static_cast<int16_t>(key), view);
        }, py::arg("key"), py::arg("value"), R"pbdoc(

        Set/replace the channel for a layer at the provided index. 
                
        :param key: The ID or index of the channel
        :type key: :class:`psapi.enum.ChannelID` | int

        :param value: The channel data with dimensions (height, width)
        :type value: np.ndarray
        
	)pbdoc");

    _imageDataLayer.def("__setitem__", [](Class& self, const Enum::ChannelID key, py::array_t<const T>& value)
        {
            auto view = from_py_array(tag::view{}, value, self.width(), self.height());
            self.set_channel(key, view);
        }, py::arg("key"), py::arg("value"), R"pbdoc(

	)pbdoc");


    _imageDataLayer.def("set_channel_by_index", [](Class& self, const int key, py::array_t<const T>& value)
        {
            auto view = from_py_array(tag::view{}, value, self.width(), self.height());
            self.set_channel(static_cast<int16_t>(key), view);
        }, py::arg("key"), py::arg("value"), R"pbdoc(

        Set/replace the channel for a layer at the provided index. 
                
        :param key: The index of the channel
        :type key: :class: int
        :param value: The channel data with dimensions (height, width)
        :type value: np.ndarray

	)pbdoc");

    _imageDataLayer.def("set_channel_by_id", [](Class& self, const Enum::ChannelID key, py::array_t<const T>& value)
        {
            auto view = from_py_array(tag::view{}, value, self.width(), self.height());
            self.set_channel(key, view);
        }, py::arg("key"), py::arg("value"), R"pbdoc(

        Set/replace the channel for a layer at the provided index. 
                
        :param key: The index of the channel
        :type key: :class:`psapi.enum.ChannelID`
        :param value: The channel data with dimensions (height, width)
        :type value: np.ndarray

	)pbdoc");

    _imageDataLayer.def("get_image_data", [](Class& self, const bool do_copy)
        {
            auto data = self.get_image_data(do_copy);
            std::unordered_map<int, py::array_t<T>> outData;
            for (auto& [key, value] : data)
            {
                outData[key.index] = to_py_array(std::move(value), self.width(), self.height());
            }
            return outData;
        }, py::arg("do_copy") = true, R"pbdoc(

        Extract all the channels of the ImageLayer into an unordered_map.
                
        :param do_copy: Defaults to true, Whether to copy the data
        :type do_copy: bool

        :return: The extracted image data
        :rtype: dict[psapi.util.ChannelIDInfo, numpy.ndarray]

	)pbdoc");


    _imageDataLayer.def("set_image_data", py::overload_cast<Class&, std::unordered_map<Enum::ChannelID, py::array_t<T>>&, const Enum::Compression>(&setImageDataFromIDMapping<T>),
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

    _imageDataLayer.def("set_image_data", py::overload_cast<Class&, std::unordered_map<int, py::array_t<T>>&, const Enum::Compression>(&setImageDataFromIntMapping<T>),
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

    _imageDataLayer.def("set_image_data", py::overload_cast<Class&, py::array_t<T>&, const Enum::Compression>(&setImageDataFromNpArray<T>),
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

    _imageDataLayer.def("set_compression", &Class::set_compression, py::arg("compression"), R"pbdoc(

        Change the compression codec of all the image channels.
                
        :param compression: The compression codec
        :type compression: :class:`psapi.enum.Compression`

	)pbdoc");

    _imageDataLayer.def_property_readonly("image_data", [](Class& self)
        {
            auto data = self.get_image_data(true);
            std::unordered_map<int, py::array_t<T>> outData;
            for (auto& [key, value] : data)
            {
                outData[key.index] = to_py_array(std::move(value), self.width(), self.height());
            }
            return outData;
        }, R"pbdoc(

	)pbdoc");

    _imageDataLayer.def_property_readonly("num_channels", [](Class& self)
        {
            return self.num_channels();
        });

    _imageDataLayer.def_property_readonly("channels", [](Class& self)
        {
            std::vector<int16_t> indices;
            const auto& img_data = self.image_data();
            for (const auto& [key, _] : img_data)
            {
                indices.push_back(key.index);
            }
            return indices;
        });
}