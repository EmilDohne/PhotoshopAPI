#pragma once

#include "Macros.h"
#include "Layer.h"
#include "LayeredFile/concepts.h"

PSAPI_NAMESPACE_BEGIN


/// This struct holds no data, we just use it to identify its type.
/// Artboards are a distinct type of group with children and a predefined size which they are clipped to.
/// Artboards may include any other type of layers, but not other artboard layers.
template <typename T>
	requires concepts::bit_depth<T>
struct ArtboardLayer : Layer<T>
{
	using Layer<T>::Layer;
	ArtboardLayer() = default;
};

extern template struct ArtboardLayer<bpp8_t>;
extern template struct ArtboardLayer<bpp16_t>;
extern template struct ArtboardLayer<bpp32_t>;

PSAPI_NAMESPACE_END