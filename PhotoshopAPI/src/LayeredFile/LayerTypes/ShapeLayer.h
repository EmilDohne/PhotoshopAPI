#pragma once

#include "Macros.h"
#include "Layer.h"
#include "LayeredFile/concepts.h"

PSAPI_NAMESPACE_BEGIN

/// This struct holds no data, we just use it to identify its type
template <typename T>
	requires concepts::bit_depth<T>
struct ShapeLayer : Layer<T>
{
	using Layer<T>::Layer;
	ShapeLayer() = default;
};

extern template struct ShapeLayer<bpp8_t>;
extern template struct ShapeLayer<bpp16_t>;
extern template struct ShapeLayer<bpp32_t>;

PSAPI_NAMESPACE_END