#pragma once

#include "Util/Enum.h"
#include "LayeredFile/LayerTypes/ImageDataMixins.h"
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
#include <variant>
#include <vector>
#include <span>


namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


/// Unlike with the mask data mixin we don't expose these as individual inheritance but instead directly instantiate the methods
/// on the classes to avoid having to implement trampoline classes and such. Therefore this class needs to be called directly by each
/// class inheriting from here. We make the assumption that by the point we get here such as when instantiating a SmartObjectLayer
/// that class has fully formed definitions of these functions
template <typename T, typename Class, typename PyClass>
	requires std::is_base_of_v<ImageDataMixin<T>, Class> && std::is_base_of_v<Layer<T>, Class>
void bind_image_data_mixin(PyClass& bound_class)
{

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bound_class.def("channel_indices", &Class::channel_indices, py::arg("include_mask") = true, R"pbdoc(

        Retrieve a list of all the channel indices.
		
		:param include_mask: Whether to include the mask channel

	)pbdoc");

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bound_class.def("num_channels", &Class::num_channels, py::arg("include_mask") = true, R"pbdoc(

        Retrieve the total number of channels held by the layer
		
		:param include_mask: Whether to include the mask channel

	)pbdoc");

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bound_class.def("get_image_data", [](Class& self) 
		{
			auto data = self.get_image_data();
			std::unordered_map<int, py::array_t<T>> out_data;
			for (auto& [key, value] : data)
			{
				if (key == MaskMixin<T>::s_mask_index.index)
				{
					out_data[key] = to_py_array(std::move(value), self.mask_width(), self.mask_height());
				}
				else
				{
					out_data[key] = to_py_array(std::move(value), self.width(), self.height());
				}
			}
			return out_data;
		}, R"pbdoc(

        Get all the channels of the layer (including masks) as a dict mapped by int : np.ndarray. This includes
		any mask channel which would be found at index -2. While all non-mask channels are guaranteed to be 
		the same size as width() * height() this does not hold true for the mask channel which would be the size
		of mask_width() and mask_height()
	

        :return: The extracted image data
        :rtype: dict[int, numpy.ndarray]

	)pbdoc");
	
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bound_class.def("__getitem__", [](Class& self, int key)
		{
			std::vector<T> data = self.get_channel(key);
			if (key == MaskMixin<T>::s_mask_index.index)
			{
				return to_py_array(std::move(data), self.mask_width(), self.mask_height());
			}
			return to_py_array(std::move(data), self.width(), self.height());
		}, py::arg("key"), R"pbdoc(

        Get the specified channel from the image data, this may also be the mask channel at index -2.
		If -2 is passed this function is identical to get_mask(). The mask channel will have the shape
		{ mask_height(), mask_width() } while any other channel will have the shape { height(), width() }.

		Generally accessing each channel individually is slower than accessing all of them with get_image_data()
		as that function is better parallelized. So if you wish to extract more than a couple channels it is recommended
		to get all of them.

		:raises ValueError: if the specified index does not exist on the layer 

        :return: The extracted channel
        :rtype: numpy.ndarray

	)pbdoc");

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	bound_class.def("get_channel_by_index", [](Class& self, int _id)
		{
			std::vector<T> data = self.get_channel(_id);
			if (_id == MaskMixin<T>::s_mask_index.index)
			{
				return to_py_array(std::move(data), self.mask_width(), self.mask_height());
			}
			return to_py_array(std::move(data), self.width(), self.height());
		}, py::arg("key"), R"pbdoc(

        Get the specified channel from the image data, this may also be the mask channel at index -2.
		If -2 is passed this function is identical to get_mask(). The mask channel will have the shape
		{ mask_height(), mask_width() } while any other channel will have the shape { height(), width() }.

		Generally accessing each channel individually is slower than accessing all of them with get_image_data()
		as that function is better parallelized. So if you wish to extract more than a couple channels it is recommended
		to get all of them.

		:param int: The key to access.

		:raises ValueError: if the specified index does not exist on the layer 

        :return: The extracted channel
        :rtype: numpy.ndarray

	)pbdoc");


	bound_class.def("get_channel_by_id", [](Class& self, Enum::ChannelID _id)
		{
			std::vector<T> data = self.get_channel(_id);
			if (_id == MaskMixin<T>::s_mask_index.id)
			{
				return to_py_array(std::move(data), self.mask_width(), self.mask_height());
			}
			return to_py_array(std::move(data), self.width(), self.height());
		}, py::arg("key"), R"pbdoc(

        Get the specified channel from the image data, this may also be the mask channel at index -2.
		If -2 is passed this function is identical to get_mask(). The mask channel will have the shape
		{ mask_height(), mask_width() } while any other channel will have the shape { height(), width() }.

		Generally accessing each channel individually is slower than accessing all of them with get_image_data()
		as that function is better parallelized. So if you wish to extract more than a couple channels it is recommended
		to get all of them.

		:param psapi.enum.ColorMode key: The key to access.

		:raises ValueError: if the specified index does not exist on the layer 

        :return: The extracted channel
        :rtype: numpy.ndarray

	)pbdoc");
}