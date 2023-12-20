#pragma once

#include "Macros.h"
#include "Layer.h"

PSAPI_NAMESPACE_BEGIN

// This struct holds no data, we just use it to identify its type
// We dont actually even store these and only use them when going from
// flat -> nested
template <typename T>
struct SectionDividerLayer : Layer<T>
{
	SectionDividerLayer() = default;

	std::tuple<LayerRecord, ChannelImageData> toPhotoshop(const Enum::ColorMode colorMode, const bool doCopy);
private:

};


PSAPI_NAMESPACE_END