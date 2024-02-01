#pragma once

#include "Macros.h"
#include "Layer.h"

PSAPI_NAMESPACE_BEGIN

/// This struct holds no data, we just use it to identify its type
/// This will probably be split into multiple files later on
template <typename T>
struct AdjustmentLayer : Layer<T>
{

	AdjustmentLayer() = default;

};

PSAPI_NAMESPACE_END