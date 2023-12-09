#pragma once

#include "Macros.h"
#include "Layer.h"

PSAPI_NAMESPACE_BEGIN

// This struct holds no data, we just use it to identify its type
template <typename T>
struct SectionDividerLayer : Layer<T>
{
	SectionDividerLayer() = default;
};


PSAPI_NAMESPACE_END