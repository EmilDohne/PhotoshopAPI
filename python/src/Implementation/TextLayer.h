#pragma once

#include "LayeredFile/LayerTypes/TextLayer/TextLayer.h"
#include "LayeredFile/LayerTypes/Layer.h"
#include "Macros.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/numpy.h>

#include <memory>
#include <string>
#include <vector>

namespace py = pybind11;
using namespace NAMESPACE_PSAPI;


// Create an alternative constructor which inlines the Layer<T>::Params since the more pythonic version is to have
// kwargs rather than a separate structure, and provides an interface for optional numpy mask arrays.
template <typename T>
std::shared_ptr<TextLayer<T>> createTextLayer(
	const std::string& layer_name,
	const std::string& text,
	const std::string& font,
	double font_size,
	const std::vector<double>& fill_color,
	double position_x,
	double position_y,
	double box_width,
	double box_height,
	const std::optional<py::array_t<T>> layer_mask,
	int width,
	int height,
	const Enum::BlendMode blend_mode,
	int pos_x,
	int pos_y,
	float opacity,
	const Enum::Compression compression,
	const Enum::ColorMode color_mode,
	bool is_visible,
	bool is_locked
)
{
	if (layer_name.size() > 255)
	{
		throw py::value_error("layer_name parameter cannot exceed a length of 255");
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

	typename Layer<T>::Params params{};
	params.name       = layer_name;
	params.blendmode  = blend_mode;
	params.center_x   = pos_x;
	params.center_y   = pos_y;
	params.width      = static_cast<uint32_t>(width);
	params.height     = static_cast<uint32_t>(height);
	params.opacity    = static_cast<uint8_t>(opacity * 255);
	params.compression = compression;
	params.colormode  = color_mode;
	params.visible    = is_visible;
	params.locked     = is_locked;

	if (layer_mask)
	{
		if (static_cast<uint64_t>(width) * height != layer_mask.value().size())
		{
			throw py::value_error("layer_mask parameter must have the same size as the layer itself (width * height)");
		}
		params.mask = std::vector<T>(layer_mask.value().data(), layer_mask.value().data() + layer_mask.value().size());
	}

	return std::make_shared<TextLayer<T>>(
		std::move(params), text, font, font_size,
		fill_color, position_x, position_y, box_width, box_height);
}
