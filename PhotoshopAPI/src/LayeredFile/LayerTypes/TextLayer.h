#pragma once

#include "Macros.h"
#include "Layer.h"
#include "LayeredFile/concepts.h"

PSAPI_NAMESPACE_BEGIN

/// This struct holds no data, we just use it to identify its type
template <typename T>
	requires concepts::bit_depth<T>
struct TextLayer : Layer<T>
{
	TextLayer() = default;

	TextLayer(const LayerRecord& layer_record, ChannelImageData& channel_image_data, const FileHeader& header)
		: Layer<T>(layer_record, channel_image_data, header)
	{
		PSAPI_LOG("Foo", "Foo");
	}

};


extern template struct TextLayer<bpp8_t>;
extern template struct TextLayer<bpp16_t>;
extern template struct TextLayer<bpp32_t>;

PSAPI_NAMESPACE_END