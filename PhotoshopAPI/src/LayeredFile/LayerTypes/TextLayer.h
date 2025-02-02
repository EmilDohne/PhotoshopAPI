#pragma once

#include "Macros.h"
#include "Layer.h"

PSAPI_NAMESPACE_BEGIN

/// This struct holds no data, we just use it to identify its type
template <typename T>
struct TextLayer : Layer<T>
{
	TextLayer() = default;
};


extern template struct TextLayer<bpp8_t>;
extern template struct TextLayer<bpp16_t>;
extern template struct TextLayer<bpp32_t>;

PSAPI_NAMESPACE_END