#pragma once

#include "Macros.h"
#include "Logger.h"

#include <vector>
#include <functional>
#include <tuple>

// If we compile with C++<20 we replace the stdlib implementation with the compatibility
// library
#if (__cplusplus < 202002L)
#include "tcb_span.hpp"
#else
#include <span>
#endif


PSAPI_NAMESPACE_BEGIN


namespace Render
{
	namespace Impl
	{
		template <typename T>
		auto spans_to_tuple(const std::vector<std::span<T>>& spans)
		{
			// Convert a vector of spans to a tuple of spans for std::apply
			return std::apply(
				[](auto&... spans) { return std::make_tuple(spans...); },
				spans
			);
		}
	} // namespace Impl

} // namespace Impl


PSAPI_NAMESPACE_END