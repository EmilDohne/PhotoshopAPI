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



template <typename T>
void setImageDataFromIntMapping(
	_ImageDataLayerType<T>& layer,
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
	_ImageDataLayerType<T>& layer,
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
	_ImageDataLayerType<T>& layer,
	py::array_t<T>& image_data,
	const Enum::Compression compression
)
{
	auto img_data_cpp = from_py_array(tag::id_mapping{}, image_data, image_data.shape(0), layer.width(), layer.height(), layer.color_mode());
	layer.set_image_data(std::move(img_data_cpp), compression);
}