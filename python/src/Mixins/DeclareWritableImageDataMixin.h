#pragma once

#include "Util/Enum.h"
#include "LayeredFile/LayerTypes/ImageDataMixins.h"
#include "PyUtil/ImageConversion.h"
#include "Macros.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>
#include <pybind11/iostream.h>

#include <unordered_map>
#include <iostream>
#include <vector>
#include <span>
#include <variant>


namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


/// Unlike with the mask data mixin we don't expose these as individual inheritance but instead directly instantiate the methods
/// on the classes to avoid having to implement trampoline classes and such. Therefore this class needs to be called directly by each
/// class inheriting from here. We make the assumption that by the point we get here such as when instantiating a SmartObjectLayer
/// that class has fully formed definitions of these functions
template <typename T, typename Class, typename PyClass>
	requires std::is_base_of_v<WritableImageDataMixin<T>, Class> && std::is_base_of_v<Layer<T>, Class>
void bind_writable_image_data_mixin(PyClass& bound_class)
{

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bound_class.def("set_image_data", [](Class& self, std::variant<py::array_t<T>, std::unordered_map<int, py::array_t<T>>>& data, std::optional<size_t> width = std::nullopt, std::optional<size_t> height = std::nullopt)
		{
			// If a width/height was passed we take that as new layer dimensions, otherwise we assume that the data passed is the same width/height as
			// the layers' width and height
			if (width && height)
			{
				self.width(width.value());
				self.height(height.value());
			}
			else if (width || height)
			{
				PSAPI_LOG_WARNING("ImageData", "Passed only width or height parameter to 'set_image_data'. Expected either none or both of these arguments to be present. Ignoring the passed argument.");
			}

			size_t _width = self.width();
			size_t _height = self.height();

			if (std::holds_alternative<py::array_t<T>>(data))
			{
				auto pyarray_data = std::get<py::array_t<T>>(data);
				auto img_data_cpp = from_py_array(tag::id_mapping{}, pyarray_data, pyarray_data.shape(0), _width, _height, self.color_mode());
				self.set_image_data(std::move(img_data_cpp));
			}
			else
			{
				auto& map_data = std::get<std::unordered_map<int, py::array_t<T>>>(data);
				// Extract the mask channel and extract its size from the array shape and not from the passed width or height as it is potentially
				// different than the rest of the image data.
				if (map_data.contains(MaskMixin<T>::s_mask_index.index))
				{
					auto shape = Util::Impl::shape_from_py_array(map_data[MaskMixin<T>::s_mask_index.index], {2}, map_data[MaskMixin<T>::s_mask_index.index].size());
					auto view = from_py_array(tag::view{}, map_data[MaskMixin<T>::s_mask_index.index], shape[0], shape[1]);
					self.set_mask(view, shape[1], shape[0]);
					map_data.erase(MaskMixin<T>::s_mask_index.index);
				}

				std::unordered_map<int, std::vector<T>> img_data_cpp;
				// Convert our image data to c++ vector data, the constructor checks for the right amount of channels
				for (auto& [key, value] : map_data)
				{
					img_data_cpp[key] = from_py_array(tag::vector{}, value, _width, _height);
				}
				self.set_image_data(std::move(img_data_cpp));
			}
			
		}, py::arg("data"), py::arg("width") = std::nullopt, py::arg("height") = std::nullopt, R"pbdoc(

        Set the image data of all the channels (may include mask channels), optionally passing in new dimensions that the 
		layer should assume (when replacing with different image data). While all channels must be identical in size, the mask
		channel (index -2) may be any other size and we will extract the dimensions from the 2d numpy array instead. 

		:param data: 
			The image data to set onto the layer, this may be a ndarray with e.g. 3 or 4 dimensions for RGB or a dict mapping the indices
			directly to individual channels. For RGB there must always be the indices 0, 1, 2 to represent the R, G and B channels and the 
			same applies to the other color modes.
		:type data: np.ndarray | dict[int, numpy.ndarray]
	
		:param width: An optional width in case the new image data does not have the same width as the layer before. If this is specified the height parameter must also be provided
		:type width: Optional[int]
		:param height: An optional height in case the new image data does not have the same height as the layer before. If this is specified the width parameter must also be provided
		:type height: Optional[int]

	)pbdoc");

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bound_class.def("__setitem__", [](Class& self, std::variant<int, Enum::ChannelID> _id, py::array_t<T>& data)
		{
			Enum::ChannelIDInfo idinfo{};
			if (std::holds_alternative<int>(_id))
			{
				idinfo = Enum::toChannelIDInfo(std::get<int>(_id), self.color_mode());
			}
			else
			{
				idinfo = Enum::toChannelIDInfo(std::get<Enum::ChannelID>(_id), self.color_mode());
			}

			if (idinfo == MaskMixin<T>::s_mask_index)
			{
				auto shape = Util::Impl::shape_from_py_array(data, { 2 }, data.size());
				auto view = from_py_array(tag::view{}, data, shape[1], shape[0]);
				self.set_mask(view, shape[1], shape[0]);
			}
			else
			{
				auto view = from_py_array(tag::view{}, data, self.width(), self.height());
				self.set_channel(idinfo.index, view);
			}
		}, py::arg("key"), py::arg("data"), R"pbdoc(

        Set/replace the channel for a layer at the provided index. This may also be the mask channel (-2).
		If the provided image data does not have the shape { height, width } or { mask_height, mask_width } this
		function raises a ValueError.
                
        :param key: The ID or index of the channel
        :type key: :class:`psapi.enum.ChannelID` | int

        :param value: The channel data with dimensions (height, width)
        :type value: np.ndarray

	)pbdoc");

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bound_class.def("set_channel_by_index", [](Class& self, int _id, py::array_t<T>& data)
		{
			Enum::ChannelIDInfo idinfo = Enum::toChannelIDInfo(_id, self.color_mode());
			if (idinfo == MaskMixin<T>::s_mask_index)
			{
				auto shape = Util::Impl::shape_from_py_array(data, {2}, data.size());
				auto view = from_py_array(tag::view{}, data, shape[1], shape[0]);
				self.set_mask(view, shape[1], shape[0]);
			}
			else
			{
				auto view = from_py_array(tag::view{}, data, self.width(), self.height());
				self.set_channel(idinfo.index, view);
			}
		}, py::arg("key"), py::arg("data"), R"pbdoc(

        Set/replace the channel for a layer at the provided index. This may also be the mask channel (-2).
		If the provided image data does not have the shape { height, width } or { mask_height, mask_width } this
		function raises a ValueError.
                
        :param key: The index of the channel
        :type key: int

        :param value: The channel data with dimensions (height, width)
        :type value: np.ndarray

	)pbdoc");

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bound_class.def("set_channel_by_id", [](Class& self, Enum::ChannelID _id, py::array_t<T>& data)
		{
			Enum::ChannelIDInfo idinfo = Enum::toChannelIDInfo(_id, self.color_mode());
			if (idinfo == MaskMixin<T>::s_mask_index)
			{
				auto shape = Util::Impl::shape_from_py_array(data, { 2 }, data.size());
				auto view = from_py_array(tag::view{}, data, shape[1], shape[0]);
				self.set_mask(view, shape[1], shape[0]);
			}
			else
			{
				auto view = from_py_array(tag::view{}, data, self.width(), self.height());
				self.set_channel(idinfo.index, view);
			}
		}, py::arg("key"), py::arg("data"), R"pbdoc(

        Set/replace the channel for a layer at the provided index. This may also be the mask channel (-2).
		If the provided image data does not have the shape { height, width } or { mask_height, mask_width } this
		function raises a ValueError.
                
        :param key: The ID of the channel
        :type key: :class:`psapi.enum.ChannelID`

        :param value: The channel data with dimensions (height, width)
        :type value: np.ndarray

	)pbdoc");
}