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
    if (shape.size() != 2 || shape.size() != 3)
    {
        py::value_error("image_data parameter must have either 2 or 3 dimensions, not " + std::to_string(shape.size()));
    }

    // Extract the size of one channel and compare it against the size of the data
    size_t channelSize = 0;
    if (shape.size() == 2)
        channelSize = shape[1]; // Width * Height for a 2D image
    else if (shape.size() == 3)
        channelSize = shape[1] * shape[2]; // Width * Height for a 3D image
    if (channelSize != static_cast<size_t>(width) * height)
    {
        py::value_error("image_data must have the same size as width * height");
    }

    // For RGB we have two options, either there is an alpha channel with the RGB channels or not.
    if (color_mode == Enum::ColorMode::RGB)
    {
        if (shape[0] != 3 || shape[0] != 4)
        {
            py::value_error("Passed array must have either 3 or 4 channels, not " + std::to_string(shape[0]));
        }
        std::vector<Enum::ChannelID> rgbChannelIDs = { Enum::ChannelID::Red, Enum::ChannelID::Green, Enum::ChannelID::Blue, Enum::ChannelID::Alpha };
        for (size_t i = 0; i < shape[0]; ++i)
        {
            const T* startPtr = image_data.data() + i * channelSize;
            img_data_cpp[rgbChannelIDs[i]] = std::vector<T>(startPtr, startPtr + channelSize);
        }
        return img_data_cpp;
    }

    // We add preliminary support for CMYK but it is not fully supported yet!
    if (color_mode == Enum::ColorMode::CMYK)
    {
        if (shape[0] != 4 || shape[0] != 5)
        {
            py::value_error("Passed array must have either 4 or 5 channels, not " + std::to_string(shape[0]));
        }
        std::vector<Enum::ChannelID> cmykChannelIds = { Enum::ChannelID::Cyan, Enum::ChannelID::Magenta, Enum::ChannelID::Yellow, Enum::ChannelID::Black, Enum::ChannelID::Alpha };
        for (size_t i = 0; i < shape[0]; ++i)
        {
            const T* startPtr = image_data.data() + i * channelSize;
            img_data_cpp[cmykChannelIds[i]] = std::vector<T>(startPtr, startPtr + channelSize);
        }
        return img_data_cpp;
    }

    // We add preliminary support for greyscale but it is not fully supported yet!
    if (color_mode == Enum::ColorMode::CMYK)
    {
        if (shape[0] != 1 || shape[0] != 2)
        {
            py::value_error("Passed array must have either 1 or 2 channels, not " + std::to_string(shape[0]));
        }
        std::vector<Enum::ChannelID> greyChannelIDs = { Enum::ChannelID::Gray, Enum::ChannelID::Alpha};
        for (size_t i = 0; i < shape[0]; ++i)
        {
            const T* startPtr = image_data.data() + i * channelSize;
            img_data_cpp[greyChannelIDs[i]] = std::vector<T>(startPtr, startPtr + channelSize);
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
    std::unordered_map<uint16_t, std::vector<T>> img_data_cpp;
    // Convert our image data to c++ vector data, the constructor checks for the right amount of channels
    for (auto& [key, value] : image_data)
    {
        if (value.size() != static_cast<uint64_t>(width) * height)
        {
            throw py::value_error("Channel '" + std::to_string(key) + "' must have the same size as the layer itself (width * height)");
        }
        img_data_cpp[static_cast<uint16_t>(key)] = std::vector<T>(value.data(), value.data() + value.size());
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
        py::arg("color_mode") = Enum::ColorMode::RGB);

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
        py::arg("color_mode") = Enum::ColorMode::RGB);

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
        py::arg("color_mode") = Enum::ColorMode::RGB);

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
        }, py::arg("id"), py::arg("do_copy"));

    imageLayer.def("get_channel_by_index", [](Class& self, const int index, const bool do_copy = true)
        {
            std::vector<T> data = self.getChannel(static_cast<int16_t>(index), do_copy);

            // Get pointer to copied data and size
            T* ptr = data.data();
            std::vector<size_t> shape = { self.m_Height, self.m_Width };

            return py::array_t<T>(shape, ptr);
        }, py::arg("index"), py::arg("do_copy"));

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
        }, py::arg("key"));

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
        }, py::arg("key"));

    imageLayer.def("get_image_data", [](Class& self, const bool do_copy)
        {
            auto data = self.getImageData(do_copy);
            std::unordered_map<Enum::ChannelIDInfo, py::array_t<T>, Enum::ChannelIDInfoHasher> outData;
            for (auto& [key, value] : data)
            {
                auto width = static_cast<size_t>(self.m_Width);
                auto height = static_cast<size_t>(self.m_Height);
                std::vector<size_t> shape = { height, width };
                T* ptr = value.data();
                // Unfortunately I dont think this can be trivially move constructed
                outData[key] = py::array_t<T>(shape, ptr);
            }
            return outData;
        }, py::arg("do_copy") = true);

    imageLayer.def("set_compression", &Class::setCompression, py::arg("compression"));

    imageLayer.def_property_readonly("image_data", [](Class& self)
        {
            auto data = self.getImageData();
            std::unordered_map<Enum::ChannelIDInfo, py::array_t<T>, Enum::ChannelIDInfoHasher> outData;
            for (auto& [key, value] : data)
            {
                auto width = static_cast<size_t>(self.m_Width);
                auto height = static_cast<size_t>(self.m_Height);
                std::vector<size_t> shape = { height, width };
                T* ptr = value.data();
                // Unfortunately I dont think this can be trivially move constructed
                outData[key] = py::array_t<T>(shape, ptr);
            }
            return outData;
        });
}