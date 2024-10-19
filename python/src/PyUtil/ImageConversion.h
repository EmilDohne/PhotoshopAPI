/*
Utility header which defines some common conversion function between py::array_t<T> <---> std::vector<T> with built-in
error checking and handling. When doing any channel conversions these functions should be preferred
*/

#pragma once

#include "Util/Enum.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>
#include <pybind11/iostream.h>

// If we compile with C++<20 we replace the stdlib implementation with the compatibility
// library
#if (__cplusplus < 202002L)
#include "tcb_span.hpp"
#else
#include <span>
#endif

#include <vector>
#include <string>
#include <algorithm>

#include <cassert>


namespace Util
{
	namespace Impl
	{

		/// Format a number to have a comma placed on every 1000s. So 1024000 would be converted to 1,024,000
		/// for easier legibility
		inline std::string format_number(size_t number)
		{
			auto num_str = std::to_string(number);
			std::string num_str_formatted;
			for (size_t i = 0; i < num_str.size(); ++i)
			{
				auto idx = num_str.size() - 1 - i;
				if (i % 3 == 0 && i != 0)
				{
					num_str_formatted += ",";
				}
				num_str_formatted += num_str.at(idx);
			}
			std::reverse(num_str_formatted.begin(), num_str_formatted.end());
			return num_str_formatted;
		}


		/// Generate a shape array from the py::array_t checking at runtime whether the shape fits into the allowed dims and matches total_size
		///
		/// \param data The data to extract the shape information from
		/// \param allowed_dims The number of dimensions that are allowed. Could e.g. be {1, 2} to allow one and two dimensional arrays
		/// \param total_size The total size of the expected data, if the dimensions do not hold this amount of data we throw a value_error
		/// 
		/// \return The shape as a cpp array
		template <typename T>
		std::vector<size_t> shape_from_py_array(const py::array_t<T>& data, const std::vector<size_t> allowed_dims, size_t total_size)
		{
			std::vector<size_t> shape;
			size_t sum = 1;
			for (size_t i = 0; i < data.ndim(); ++i)
			{
				shape.push_back(data.shape(i));
				sum *= data.shape(i);
			}

			// Check that the shape is within the allowed dimensions
			if (std::find(allowed_dims.begin(), allowed_dims.end(), shape.size()) == allowed_dims.end())
			{
				std::string error_msg = "Invalid number of dimensions received, array must have one of the following number of dimensions: { ";
				for (size_t i = 0; i < allowed_dims.size() - 1; ++i)
				{
					error_msg += std::to_string(allowed_dims[i]), ", ";
				}
				error_msg += std::to_string(allowed_dims.back());
				error_msg += " }. Instead got: " + std::to_string(shape.size());
				throw py::value_error(error_msg);
			}

			if (sum != total_size)
			{
				throw py::value_error(
					"Invalid array size received, expected " + format_number(total_size) + " but instead got " + \
					format_number(sum)
					);
			}
			return shape;
		}


		/// Calculate the byte strides from a cpp vector with its corresponding shape vector.
		template <typename T>
		std::vector<size_t> strides_from_shape(std::vector<size_t> shape)
		{
			std::vector<size_t> strides(shape.size());

			if (shape.empty()) 
			{
				return strides; // Return empty strides if shape is empty
			}

			size_t stride = sizeof(T);  // Start with the size of the element in bytes
			for (int i = shape.size() - 1; i >= 0; --i) 
			{
				strides[i] = stride;
				stride *= shape[i];  // Move to the next dimension stride in terms of element count
			}
			return strides;
		}


		/// Generate channel indices from the number of expected channels as well as the colormode. What this means
		/// is for example for RGB data we automatically want to forward the last channel to be the index of
		/// the alpha channel (-1).
		/// 
		/// This function currently maps RGB, CMYK and Grayscale color modes
		inline std::vector<int16_t> generate_channel_indices(size_t expected_channels, Enum::ColorMode color_mode)
		{
			if (color_mode == Enum::ColorMode::RGB)
			{
				if (expected_channels == 3)
				{
					return { 0, 1, 2 };
				}
				if (expected_channels == 4)
				{
					return { 0, 1, 2, -1 };
				}
				throw py::value_error(
					"Invalid number of channels provided for '" + Enum::colorModeToString(color_mode) + \
					"' colormode, expected 3 or 4 but got " + std::to_string(expected_channels)
				);
			}
			else if (color_mode == Enum::ColorMode::CMYK)
			{
				if (expected_channels == 4)
				{
					return { 0, 1, 2, 3 };
				}
				if (expected_channels == 5)
				{
					return { 0, 1, 2, 3, -1 };
				}
				throw py::value_error(
					"Invalid number of channels provided for '" + Enum::colorModeToString(color_mode) + \
					"' colormode, expected 4 or 5 but got " + std::to_string(expected_channels)
				);
			}
			else if (color_mode == Enum::ColorMode::Grayscale)
			{
				if (expected_channels == 1)
				{
					return { 0 };
				}
				if (expected_channels == 2)
				{
					return { 0, -1 };
				}
				throw py::value_error(
					"Invalid number of channels provided for '" + Enum::colorModeToString(color_mode) + \
					"' colormode, expected 1 or 2 but got " + std::to_string(expected_channels)
				);
			}
			else
			{
				throw py::value_error(
					"Invalid colormode '" + Enum::colorModeToString(color_mode) + "' provided, unable to compute" \
					"channel indices from it."
				);
			}
		}

		namespace 
		{

			inline void check_shape_1d(std::vector<size_t> shape, size_t expected_width, size_t expected_height)
			{
				assert(shape.size() == 1);
				if (shape[0] != expected_height * expected_width)
				{
					throw py::value_error("Invalid 1st dimension size encounted, expected " + format_number(expected_height * expected_width) + \
						" but instead got " + format_number(shape[0]));
				}
			}

			inline void check_shape_2d(std::vector<size_t> shape, size_t expected_width, size_t expected_height)
			{
				assert(shape.size() == 2);
				if (shape[0] != expected_height)
				{
					throw py::value_error("Invalid 1st dimension size encounted, expected " + format_number(expected_height) + \
						" but instead got " + format_number(shape[0]) + ". This number should represent the layers' height");
				}
				if (shape[1] != expected_width)
				{
					throw py::value_error("Invalid 2nd dimension size encounted, expected " + format_number(expected_width) + \
						" but instead got " + format_number(shape[1]) + ". This number should represent the layers' width");
				}
			}

			inline void check_shape_3d(std::vector<size_t> shape, size_t expected_channels, size_t expected_width, size_t expected_height)
			{
				assert(shape.size() == 3);
				if (shape[0] != expected_height)
				{
					throw py::value_error("Invalid 1st dimension size encounted, expected " + format_number(expected_channels) + \
						" but instead got " + format_number(shape[0]) + ". This number should represent the layers' number of channels");
				}
				if (shape[1] != expected_height)
				{
					throw py::value_error("Invalid 2nd dimension size encounted, expected " + format_number(expected_height) + \
						" but instead got " + format_number(shape[1]) + ". This number should represent the layers' height");
				}
				if (shape[2] != expected_width)
				{
					throw py::value_error("Invalid 3rd dimension size encounted, expected " + format_number(expected_width) + \
						" but instead got " + format_number(shape[2]) + ". This number should represent the layers' width");
				}
			}
		
		}

		/// Check that the shape vector matches the expected format. I.e. for a 2d vector it checks whether the height is the
		/// 1st dimension as expected or if this is swapped which would cause the output data to be weird
		inline void check_shape(std::vector<size_t> shape, size_t expected_width, size_t expected_height, size_t expected_channels = 1)
		{
			if (shape.size() == 1)
			{
				check_shape_1d(shape, expected_width, expected_height);
			}
			else if (shape.size() == 2)
			{
				check_shape_2d(shape, expected_width, expected_height);
			}
			else if (shape.size() == 3)
			{
				check_shape_3d(shape, expected_channels, expected_width, expected_height);
			}
			else
			{
				throw py::value_error("Invalid number of array dimensions encountered, expected 1, 2 or 3 but instead got " + std::to_string(shape.size()));
			}
		}

		/// Check if the provided python array is c-style contiguous and if it isn't force this conversion in place
		/// alerting the user of this modification. This will touch the underlying numpy array
		template <typename T>
		void check_c_style_contiguous(py::array_t<T>& data)
		{
			if (py::detail::npy_api::constants::NPY_ARRAY_C_CONTIGUOUS_ != (data.flags() & py::detail::npy_api::constants::NPY_ARRAY_C_CONTIGUOUS_))
			{
				PSAPI_LOG_WARNING("Python", "Provided image data was detected to not be c-style contiguous, forcing this conversion in-place");
				data = data.template cast<py::array_t<T, py::array::c_style | py::array::forcecast>>();
			}
		}

		/// Check that the python array passed is not null
		template <typename T>
		void check_not_null(const py::array_t<T>& data)
		{
			if (data.data() == nullptr)
			{
				throw py::value_error(
					"Python numpy array passed to function resolves to nullptr. If you believe this to be a mistake" \
					" please open a ticket on the projects' github page."
				);
			}
		}
	
		/// Check that the given span matches the overall size of the shape vector provided
		template <typename T>
		void check_cpp_span_matches_shape(const std::span<const T> data, std::vector<size_t> shape)
		{
			size_t sum = 1;
			for (const auto item : shape)
			{
				sum *= item;
			}

			if (sum != data.size())
			{
				std::string error_msg = "Invalid array dimension received: { ";
				for (size_t i = 0; i < shape.size() - 1; ++i)
				{
					error_msg += std::to_string(shape[i]), ", ";
				}
				error_msg += std::to_string(shape.back());
				error_msg += " }. Expected these to sum up to " + format_number(data.size()) + " but they instead sum up to ";
				error_msg += format_number(sum) + ". This could be due to the layers' width and height not matching the channel data";
				throw py::value_error(error_msg);
			}
		}

		/// Check that the given vector matches the overall size of the shape vector provided
		template <typename T>
		void check_cpp_vec_matches_shape(const std::vector<T>& data, std::vector<size_t> shape)
		{
			std::span<const T> data_span(data.data(), data.size());
			check_cpp_span_matches_shape(data_span, shape);
		}

	}


	/// Generate a vector from the python np array copying the data into the new container
	/// Generates a flat vector over a 1 or 2d input array. If the incoming data is not contiguous we forcecast
	/// to c-style ordering as well as asserting that the data matches expected_size
	template <typename T>
	std::vector<T> vector_from_py_array(py::array_t<T>& data, size_t expected_width, size_t expected_height)
	{
		size_t expected_size = expected_height * expected_width;
		// This checks that the size matches so we can safely construct assume expected_size
		// is the actual size from this point onwards
		auto shape = Impl::shape_from_py_array(data, { 1, 2 }, expected_size);
		Impl::check_shape(shape, expected_width, expected_height);
		Impl::check_c_style_contiguous(data);
		Impl::check_not_null(data);

		// Finally convert the channel to a cpp vector and return
		std::vector<T> data_vec(expected_size);
		std::memcpy(data_vec.data(), data.data(), expected_size * sizeof(T));
		return data_vec;
	}


	/// Generate an image data mapping from a 3-dimensional python array. The resulting map will
	/// have its alpha channel automatically forwarded from the last channel index to -1.
	/// 
	/// \param data The data we are decoding into a map. Must be 2- or 3-dimensional but 3-dimensional is preferred as it gives better information
	/// \param expected_channels how many channels we expect to have in the array
	/// \param expected_width The width of the channels
	/// \param expected_height The height of the channels
	/// \param color_mode The color mode of the file/layer which is required for deferring the channel indices
	template <typename T>
	std::unordered_map<int16_t, std::vector<T>> int_map_from_py_array(
		py::array_t<T>& data, 
		size_t expected_channels, 
		size_t expected_width, 
		size_t expected_height,
		Enum::ColorMode color_mode)
	{
		size_t expected_size = expected_channels * expected_height * expected_width;
		// This checks that the size matches so we can safely construct assume expected_size
		// is the actual size from this point onwards
		auto shape = Impl::shape_from_py_array(data, { 2, 3 }, expected_size);
		if (shape.size() == 2)
		{
			// We cheat the function a little bit here by making passing the number of channels
			// as height and the channel size as width so that an array of lets say (3, 1024)
			// would still be recognized as working. Do note that in a 3d array we have more information
			// so that should be preferred by users as then we can tell if e.g. the width and height
			// are not in the right location
			Impl::check_shape(shape, expected_height * expected_width, expected_channels);
		}
		else if (shape.size() == 3)
		{
			Impl::check_shape(shape, expected_width, expected_height, expected_channels);
		}
		Impl::check_c_style_contiguous(data);
		Impl::check_not_null(data);

		// Generate indices automatically forwarding the "excess" channels to alpha
		auto indices = Impl::generate_channel_indices(expected_channels, color_mode);
		std::unordered_map<int16_t, std::vector<T>> res;

		size_t offset = 0;
		for (auto index : indices)
		{
			size_t expected_channel_size = expected_height * expected_width;
			// Apply bounds checking per channel to ensure we dont try to access memory
			// outside of bounds
			if (offset + expected_channel_size > data.size())
			{
				throw py::value_error(
					"Image data access at channel index " + std::to_string(index) + " would exceed the" \
					"python arrays' max size. Was trying to access logical index " + Impl::format_number(offset + expected_channel_size) + \
					" but the array size is " + Impl::format_number(data.size()) + "."
				);
			}

			// Due to us forcecasting to c-style contiguous earlier we are guaranteed to have a flat array
			std::vector<T> data_vec(expected_channel_size);
			std::memcpy(data_vec.data(), data.data() + offset, expected_channel_size * sizeof(T));
			res[index] = std::move(data_vec);

			offset += expected_channel_size;
		}
		return res;
	}

	/// Generate an image data mapping from a 3-dimensional python array. The resulting map will
	/// have its alpha channel automatically forwarded from the last channel index to -1.
	/// 
	/// \param data The data we are decoding into a map. Must be 2- or 3-dimensional but 3-dimensional is preferred as it gives better information
	/// \param expected_channels how many channels we expect to have in the array
	/// \param expected_width The width of the channels
	/// \param expected_height The height of the channels
	/// \param color_mode The color mode of the file/layer which is required for deferring the channel indices
	template <typename T>
	std::unordered_map<Enum::ChannelID, std::vector<T>> id_map_from_py_array(
		py::array_t<T>& data,
		size_t expected_channels,
		size_t expected_width,
		size_t expected_height,
		Enum::ColorMode color_mode)
	{
		// Just call the int mapping function and move into a Enum::ChannelID mapped map instead
		auto mapping = int_map_from_py_array(data, expected_channels, expected_width, expected_height, color_mode);
		std::unordered_map<Enum::ChannelID, std::vector<T>> res;
		for (auto& [key, value] : mapping)
		{
			auto idinfo = Enum::toChannelIDInfo(key, color_mode);
			res[idinfo.id] = std::move(value);
		}
		return res;
	}


	/// Generate a view over the data from the python array. The span should only be used
	/// for immediate construction as memory management is not guaranteed. Generates a flat 
	/// view over a 1 or 2d input array. If the incoming data is not contiguous we forcecast
	/// to c-style ordering as well as asserting that the data matches expected_size
	/// 
	/// \param data The python numpy based array we want to create a view over
	/// \param expected_size The expected size in number of elements, NOT bytes.
	template <typename T>
	const std::span<const T> view_from_py_array(py::array_t<T>& data, size_t expected_width, size_t expected_height)
	{
		size_t expected_size = expected_height * expected_width;
		// This checks that the size matches so we can safely construct assume expected_size
		// is the actual size from this point onwards
		auto shape = Impl::shape_from_py_array(data, { 1, 2 }, expected_size);
		Impl::check_shape(shape, expected_width, expected_height);
		Impl::check_c_style_contiguous(data);
		Impl::check_not_null(data);

		// Finally convert the channel to a cpp span and return
		std::span<const T> data_span(data.data(), expected_size);
		return data_span;
	}

	/// Generate a py::array_t from std::vector copying the data into 
	/// its internal buffer.
	/// 
	/// \param data The vector to copy the data from
	/// \param shape The shape to assign to the output container
	template <typename T>
	py::array_t<T> py_array_from_vector(const std::vector<T>& data, std::vector<size_t> shape)
	{
		Impl::check_cpp_vec_matches_shape(data, shape);
		return py::array_t<T>(shape, data.data());
	}


	/// Generate a py::array_t from std::vector move constructing the data
	/// 
	/// \param data The vector to copy the data from
	/// \param shape The shape to assign to the output container
	template <typename T>
	py::array_t<T> py_array_from_vector(std::vector<T>&& data, std::vector<size_t> shape)
	{
		Impl::check_cpp_vec_matches_shape(data, shape);
		auto strides = Impl::strides_from_shape<T>(shape);

		// We generate a temporary unique_ptr to assign to the capsule
		// so that the array_t can take ownership over our data
		auto data_raw_ptr = data.data();
		auto data_ptr = std::make_unique<std::vector<T>>(std::move(data));
		auto capsule = pybind11::capsule(data_ptr.get(), [](void* p)
			{
				std::unique_ptr<std::vector<T>>(reinterpret_cast<decltype(data_ptr)::element_type*>(p));
			});
		data_ptr.release();
		// Implicitly convert from py::array to py::array_t as they inherit from one another
		return py::array(shape, strides, data_raw_ptr, capsule);
	}

	/// Generate a py::array_t from std::vector copying the data into 
	/// its internal buffer.
	/// 
	/// \param data The span to copy the data from
	/// \param shape The shape to assign to the output container
	template <typename T>
	py::array_t<T> py_array_from_view(const std::span<const T> data, std::vector<size_t> shape)
	{
		Impl::check_cpp_span_matches_shape(data, shape);
		return py::array_t<T>(shape, data.data());
	}


	/// Generate a py::array_t from an unordered_map copying the data into 
	/// its internal buffer.
	/// 
	/// \param data The map to copy the data from
	/// \param shape The shape to assign to the output container
	template <typename T, typename Key>
	py::array_t<T> py_array_from_map(const std::unordered_map<Key, std::vector<T>>& data, std::vector<size_t> shape)
	{
		assert(shape.size() == 3);
		std::vector<size_t> shape_2d = { shape[1], shape[2] };

		// For all channels check if the vector matches the shape it is supposed to and if not we instead rethrow
		// the exception giving additional channel information
		size_t count = 0;
		for (const auto& [key, value] : data)
		{
			try
			{
				Impl::check_cpp_vec_matches_shape(value, shape_2d);
			}
			catch (const py::value_error& error)
			{
				if constexpr (std::is_same_v<Key, int16_t>)
				{
					throw py::value_error("Error while parsing channel " + std::to_string(key) + ": " + error.what());
				}
				else if constexpr (std::is_same_v<Key, Enum::ChannelID>)
				{
					throw py::value_error("Error while parsing channel " + Enum::channelIDToString(key) + ": " + error.what());
				}
				else
				{
					throw py::value_error("Error while parsing channel at logical index " + std::to_string(count) + ": " + error.what());
				}
			}
			++count;
		}
		return py::array_t<T>(shape, data.data());
	}
}


/*
Miscellaneous utility functions that convert to and from py::array_t handling 1D, 2D and 3D cases overloading
based on the provided arguments.
*/


/// Keys for tag dispatching
namespace tag
{
	struct int_mapping {};
	struct id_mapping {};
	struct view {};
	struct vector {};
}

template <typename T>
std::unordered_map<Enum::ChannelID, std::vector<T>> from_py_array(
	tag::id_mapping _,
	py::array_t<T>& data,
	size_t expected_channels,
	size_t expected_width,
	size_t expected_height,
	Enum::ColorMode color_mode
)
{
	return Util::id_map_from_py_array(data, expected_channels, expected_width, expected_height, color_mode);
}

template <typename T>
std::unordered_map<int16_t, std::vector<T>> from_py_array(
	tag::int_mapping _,
	py::array_t<T>& data,
	size_t expected_channels,
	size_t expected_width,
	size_t expected_height,
	Enum::ColorMode color_mode
)
{
	return Util::int_map_from_py_array(data, expected_channels, expected_width, expected_height, color_mode);
}


/// ONLY use this if you plan to use this data directly but don't count on this data to exist after 
/// we exit the cpp scope again!
template <typename T>
const std::span<const T> from_py_array(
	tag::view _,
	py::array_t<T>& data,
	size_t expected_width,
	size_t expected_height)
{
	return Util::view_from_py_array(data, expected_width, expected_height);
}


template <typename T>
std::vector<T> from_py_array(
	tag::vector _,
	py::array_t<T>& data,
	size_t expected_width,
	size_t expected_height)
{
	return Util::vector_from_py_array(data, expected_width, expected_height);
}


template <typename T>
py::array_t<T> to_py_array(const std::span<const T> data, size_t width, size_t height)
{
	std::vector<size_t> shape{ height, width };
	return Util::py_array_from_view(data, shape);
}


template <typename T>
py::array_t<T> to_py_array(const std::vector<T>& data, size_t width, size_t height)
{
	std::vector<size_t> shape{ height, width };
	return Util::py_array_from_vector(data, shape);
}


template <typename T>
py::array_t<T> to_py_array(std::vector<T>&& data, size_t width, size_t height)
{
	std::vector<size_t> shape{ height, width };
	return Util::py_array_from_vector(std::move(data), shape);
}


template <typename T>
py::array_t<T> to_py_array(const std::unordered_map<int16_t, std::vector<T>>& data, size_t width, size_t height)
{
	std::vector<size_t> shape{ data.size(), height, width };
	return Util::py_array_from_map(data, shape);
}


template <typename T>
py::array_t<T> to_py_array(const std::unordered_map<Enum::ChannelID, std::vector<T>>& data, size_t width, size_t height)
{
	std::vector<size_t> shape{ data.size(), height, width };
	return Util::py_array_from_map(data, shape);
}