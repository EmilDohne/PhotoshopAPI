#pragma once

#include "Macros.h"
#include "Layer.h"

PSAPI_NAMESPACE_BEGIN

/// This struct holds no data, we just use it to identify its type.
/// We could hold references here 
template <typename T>
struct SmartObjectLayer : Layer<T>
{
	SmartObjectLayer() = default;
};


PSAPI_NAMESPACE_END