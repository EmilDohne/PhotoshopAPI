#pragma once

#include "Macros.h"
#include "LayeredFile/concepts.h"

PSAPI_NAMESPACE_BEGIN

/// Forward declaration of TextLayer so that CRTP-style mixins can
/// static_cast<TextLayer<T>*>(this) without requiring the full definition.
template <typename T>
	requires concepts::bit_depth<T>
struct TextLayer;

PSAPI_NAMESPACE_END
