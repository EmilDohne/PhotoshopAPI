#pragma once


#include "LayeredFile/LayerTypes/ImageLayer.h"
#include "LayeredFile/LayerTypes/Layer.h"
#include "Util/Enum.h"
#include "PyUtil/ImageConversion.h"
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


// Create an alternative constructor which inlines the Layer<T>::Params since the more pythonic version would be to have kwargs rather 
// than a separate structure as well as creating an interface for numpy.
template <typename T>
std::shared_ptr<ImageLayer<T>> createImageLayerFromNpArray(
	py::array_t<T>& image_data,
	const std::string& layer_name,
	const std::optional<py::array_t<T>> layer_mask,
	int width,
	int height,
	const Enum::BlendMode blend_mode,
	int pos_x,
	int pos_y,
	int opacity,
	const Enum::Compression compression,
	const Enum::ColorMode color_mode,
	bool is_visible,
	bool is_locked
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
		params.mask = std::vector<T>(layer_mask.value().data(), layer_mask.value().data() + layer_mask.value().size());
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
	auto img_data_cpp = from_py_array(tag::id_mapping{}, image_data, image_data.shape(0), width, height, color_mode);

	params.name = layer_name;
	params.blendmode = blend_mode;
	params.center_x = pos_x;
	params.center_y = pos_y;
	params.width = width;
	params.height = height;
	params.opacity = opacity;
	params.compression = compression;
	params.colormode = color_mode;
	params.visible = is_visible;
	params.locked = is_locked;
	return std::make_shared<ImageLayer<T>>(std::move(img_data_cpp), params);
}


// Create an alternative constructor which inlines the Layer<T>::Params since the more pythonic version would be to have kwargs rather 
// than a separate structure as well as creating an interface for numpy.
template <typename T>
std::shared_ptr<ImageLayer<T>> createImageLayerFromIDMapping(
	std::unordered_map<Enum::ChannelID, py::array_t<T>>& image_data,
	const std::string& layer_name,
	const std::optional<py::array_t<T>> layer_mask,
	int width,
	int height,
	const Enum::BlendMode blend_mode,
	int pos_x,
	int pos_y,
	int opacity,
	const Enum::Compression compression,
	const Enum::ColorMode color_mode,
	bool is_visible,
	bool is_locked
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
		params.mask = std::vector<T>(layer_mask.value().data(), layer_mask.value().data() + layer_mask.value().size());
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
		img_data_cpp[key] = from_py_array(tag::vector{}, value, width, height);
	}

	params.name = layer_name;
	params.blendmode = blend_mode;
	params.center_x = pos_x;
	params.center_y = pos_y;
	params.width = width;
	params.height = height;
	params.opacity = opacity;
	params.compression = compression;
	params.colormode = color_mode;
	params.visible = is_visible;
	params.locked = is_locked;
	return std::make_shared<ImageLayer<T>>(std::move(img_data_cpp), params);
}


// Create an alternative constructor which inlines the Layer<T>::Params since the more pythonic version would be to have kwargs rather 
// than a separate structure as well as creating an interface for numpy.
template <typename T>
std::shared_ptr<ImageLayer<T>> createImageLayerFromIntMapping(
	std::unordered_map<int, py::array_t<T>>& image_data,
	const std::string& layer_name,
	const std::optional<py::array_t<T>> layer_mask,
	int width,
	int height,
	const Enum::BlendMode blend_mode,
	int pos_x,
	int pos_y,
	int opacity,
	const Enum::Compression compression,
	const Enum::ColorMode color_mode,
	bool is_visible,
	bool is_locked
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
		params.mask = std::vector<T>(layer_mask.value().data(), layer_mask.value().data() + layer_mask.value().size());
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
		img_data_cpp[static_cast<int16_t>(key)] = from_py_array(tag::vector{}, value, width, height);
	}

	params.name = layer_name;
	params.blendmode = blend_mode;
	params.center_x = pos_x;
	params.center_y = pos_y;
	params.width = width;
	params.height = height;
	params.opacity = opacity;
	params.compression = compression;
	params.colormode = color_mode;
	params.visible = is_visible;
	params.locked = is_locked;
	return std::make_shared<ImageLayer<T>>(std::move(img_data_cpp), params);
}



template <typename T>
void setImageDataFromIntMapping(
	ImageLayer<T>& layer,
	std::unordered_map<int, py::array_t<T>>& image_data,
	const Enum::Compression compression
)
{
	std::unordered_map<int16_t, std::vector<T>> img_data_cpp;
	// Convert our image data to c++ vector data, the constructor checks for the right amount of channels
	for (auto& [key, value] : image_data)
	{
		img_data_cpp[static_cast<int16_t>(key)] = from_py_array(tag::vector{}, value, layer.width(), layer.height());
	}
	layer.set_image_data(std::move(img_data_cpp), compression);
}


template <typename T>
void setImageDataFromIDMapping(
	ImageLayer<T>& layer,
	std::unordered_map<Enum::ChannelID, py::array_t<T>>& image_data,
	const Enum::Compression compression
)
{
	std::unordered_map<Enum::ChannelID, std::vector<T>> img_data_cpp;
	// Convert our image data to c++ vector data, the constructor checks for the right amount of channels
	for (auto& [key, value] : image_data)
	{
		img_data_cpp[key] = from_py_array(tag::vector{}, value, layer.width(), layer.height());
	}
	layer.set_image_data(std::move(img_data_cpp), compression);
}


template <typename T>
void setImageDataFromNpArray(
	ImageLayer<T>& layer,
	py::array_t<T>& image_data,
	const Enum::Compression compression
)
{
	auto img_data_cpp = from_py_array(tag::id_mapping{}, image_data, image_data.shape(0), layer.width(), layer.height(), layer.color_mode());
	layer.set_image_data(std::move(img_data_cpp), compression);
}