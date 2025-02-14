#pragma once

#include "LayeredFile/LayerTypes/GroupLayer.h"
#include "LayeredFile/LayerTypes/Layer.h"
#include "Macros.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>
#include <pybind11/iostream.h>

#include <memory>


// Create an alternative constructor which inlines the Layer<T>::Params since the more pythonic version would be to have kwargs rather 
// than a separate structure as well as creating an interface for numpy
template <typename T>
std::shared_ptr<GroupLayer<T>> createGroupLayer(
	const std::string& layer_name,
	const std::optional<py::array_t<T>> layer_mask,
	int width,  // This is only relevant if a layer mask is set
	int height, // This is only relevant if a layer mask is set
	const Enum::BlendMode blend_mode,
	int pos_x, // This is only relevant if a layer mask is set
	int pos_y, // This is only relevant if a layer mask is set
	float opacity,
	const Enum::Compression compression,
	const Enum::ColorMode color_mode,
	bool is_collapsed,
	bool is_visible,
	bool is_locked
)
{
	typename Layer<T>::Params params{};
	// Do some preliminary checks since python has no concept of e.g. unsigned integers (without ctypes) 
	// so we must ensure the range ourselves
	if (layer_name.size() > 255)
	{
		throw py::value_error("layer_name parameter cannot exceed a length of 255");
	}
	if (layer_mask)
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
	if (opacity < 0.0f || opacity > 1.0f)
	{
		throw py::value_error("opacity must be between 0-1, got " + std::to_string(opacity));
	}

	params.name = layer_name;
	params.blendmode = blend_mode;
	params.center_x = pos_x;
	params.center_y = pos_y;
	params.width = width;
	params.height = height;
	params.opacity = static_cast<uint8_t>(opacity * 255);
	params.compression = compression;
	params.colormode = color_mode;
	params.visible = is_visible;
	params.locked = is_locked;
	return std::make_shared<GroupLayer<T>>(params, is_collapsed);
}

