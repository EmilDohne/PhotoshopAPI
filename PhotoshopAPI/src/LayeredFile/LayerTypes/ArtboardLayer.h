#pragma once

#include "Macros.h"
#include "Layer.h"

PSAPI_NAMESPACE_BEGIN


/// This struct holds no data, we just use it to identify its type.
/// Artboards are a distinct type of group with children and a predefined size which they are clipped to.
/// Artboards may include any other type of layers, but not other artboard layers.
template <typename T, typename = std::enable_if_t<
	std::is_same_v<T, uint8_t> ||
	std::is_same_v<T, uint16_t> ||
	std::is_same_v<T, float32_t>>>
struct ArtboardLayer : Layer<T>
{
	ArtboardLayer() = default;
};

PSAPI_NAMESPACE_END