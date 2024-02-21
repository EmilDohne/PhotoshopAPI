#include "LayeredFile/LayerTypes/ImageLayer.h"
#include "Macros.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>
#include <pybind11/iostream.h>

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
        ----------

        image_data : dict[:class:`psapi.util.ImageChannel`]
            A dictionary of the image data mapped by :class:`psapi.util.ChannelIDInfo`
        name : str
            The name of the layer, cannot be longer than 254
        layer_mask : LayerMask_8bit
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
    imageLayer.def("get_channel_by_id", [](Class& self, const Enum::ChannelID channel_id, const bool do_copy = true)
        {
            std::vector<T> data = self.getChannel(channel_id, do_copy);

            // Get pointer to copied data and size
            T* ptr = data.data();
            std::vector<size_t> shape = { self.m_Height, self.m_Width };

            return py::array_t<T>(shape, ptr);
        }, py::arg("channel_id"), py::arg("do_copy"),
        R"pbdoc(
                Extract a specified channel from the layer given its channel ID.
                
                :param channel_id: The ID of the channel
                :type channel_id: :class:`psapi.enum.ChannelID`

                :param do_copy: Whether to copy the data on extraction (if false the channel is invalidated)
                :type do_copy: bool            

                :return: The extracted channel
                :rtype: numpy.ndarray
            )pbdoc");
    imageLayer.def("get_channel_by_index", [](Class& self, const int channel_index, const bool do_copy = true)
        {
            std::vector<T> data = self.getChannel(static_cast<int16_t>(channel_index), do_copy);

            // Get pointer to copied data and size
            T* ptr = data.data();
            std::vector<size_t> shape = { self.m_Height, self.m_Width };

            return py::array_t<T>(shape, ptr);
        }, py::arg("channel_index"), py::arg("do_copy"),
        R"pbdoc(
                Extract a specified channel from the layer given its channel index.
                
                :param channel_index: The index of the channel
                :type channel_index: int

                :param do_copy: Whether to copy the data on extraction (if false the channel is invalidated)
                :type do_copy: bool            

                :return: The extracted channel with dimensions (height, width)
                :rtype: numpy.ndarray
            )pbdoc");
    imageLayer.def("__getitem__", [](Class& self, const Enum::ChannelID channel_id)
        {
            std::vector<T> data = self.getChannel(channel_id, true);

            // Get pointer to copied data and size
            T* ptr = data.data();
            std::vector<size_t> shape = { self.m_Height, self.m_Width };

            if (data.size() != self.m_Height * self.m_Width)
            {
                throw py::key_error("Unable to retrieve channel");
            }

            return py::array_t<T>(shape, ptr);
        }, py::arg("channel_id"),
            R"pbdoc(
                Extract a specified channel from the layer given its channel index.
                
                :param channel_id: The ID of the channel
                :type channel_id: :class:`psapi.enum.ChannelID`

                :return: The extracted channel with dimensions (height, width)
                :rtype: np.ndarray
            )pbdoc");
    imageLayer.def("__getitem__", [](Class& self, const int channel_index)
        {
            std::vector<T> data = self.getChannel(channel_index, true);

            // Get pointer to copied data and size
            T* ptr = data.data();
            std::vector<size_t> shape = { self.m_Height, self.m_Width };

            if (data.size() != self.m_Height * self.m_Width)
            {
                throw py::key_error("Unable to retrieve channel index " + std::to_string(channel_index));
            }

            return py::array_t<T>(shape, ptr);
        }, py::arg("channel_index"),
        R"pbdoc(
                :param channel_index: The index of the channel
                :type channel_index: int
            )pbdoc");
    imageLayer.def("get_image_data", &Class::getImageData,
            R"pbdoc(
                Extract all the channels of the ImageLayer into an unordered_map.
                
                :param do_copy: Whether to copy the data
                :return: The extracted image data
            )pbdoc");
    imageLayer.def("set_compression", &Class::setCompression,
            R"pbdoc(
                Change the compression codec of all the image channels.
                
                :param comp_code: The compression codec
                :type comp_code: :class:`psapi.enum.Compression`
            )pbdoc");
}