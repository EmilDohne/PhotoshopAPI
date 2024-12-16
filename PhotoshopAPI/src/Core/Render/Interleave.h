#pragma once

#include "Macros.h"
#include "Util/Logger.h"

#include <vector>
#include <algorithm>
#include <execution>
#include <ranges>

// If we compile with C++<20 we replace the stdlib implementation with the compatibility
// library
#if (__cplusplus < 202002L)
#include "tcb_span.hpp"
#else
#include <span>
#endif

#include "Implementation.h"


PSAPI_NAMESPACE_BEGIN

namespace Render
{


	/// Interleave the spans into the unified buffer taking any number of input spans (could be channels).
	///
	/// could be called like this for example:
	/// 
	/// \code{.cpp}
	/// std::vector<T> interleaved(...);
	/// std::span<T> channel_r;
	/// std::span<T> channel_g;
	/// std::span<T> channel_b;
	/// std::span<T> channel_a;
	/// 
	/// Render::interleave(interleaved, channel_r, channel_g, channel_b, channel_a);
	/// \endcode
	/// 
	/// \tparam T The data type to interleave
	/// \tparam ...Spans variadic number of input spans.
	/// 
	/// \param buffer The preallocated buffer which is intended to hold all the incoming spans. Must be exactly the size of spans `first` and `rest` combined
	/// \param first  The first span to interleave, there must be at least one of these
	/// \param rest	  The other spans to interleave, there may be any number of spans here. They must all be the exact size of `first.size()`
	template <typename T, typename... Spans> 
		requires (std::same_as<Spans, std::span<const T>> && ...)
	void interleave(std::span<T> buffer, const std::span<const T> first, const Spans... rest)
	{
		PSAPI_PROFILE_FUNCTION();
		std::array<std::span<const T>, sizeof...(rest) + 1> spans{ first, rest... };

		for (const auto& span : spans)
		{
			if (span.size() != first.size())
			{
				throw std::invalid_argument("Interleave: All input spans must have the same size.");
			}
		}

		if (buffer.size() != first.size() * spans.size())
		{
			throw std::invalid_argument("Interleave: Provided buffer is not large enough to hold all the elements to interleave.");
		}

		auto indices = std::views::iota(static_cast<std::size_t>(0), first.size());
		std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [&buffer, &spans, &first](auto idx)
			{
				std::size_t start_idx = spans.size() * idx;

				for (size_t i = 0; i < spans.size(); ++i)
				{
					buffer[start_idx + i] = spans[i][idx];
				}
			});
	}

	/// Interleave the spans into a unified buffer taking any number of input spans (could be channels).
	///
	/// could be called like this for example:
	/// 
	/// \code{.cpp}
	/// std::span<T> channel_r;
	/// std::span<T> channel_g;
	/// std::span<T> channel_b;
	/// std::span<T> channel_a;
	/// 
	/// std::vector<T> interleaved = Render::interleave(channel_r, channel_g, channel_b, channel_a);
	/// \endcode
	/// 
	/// \tparam T The data type to interleave
	/// \tparam ...Spans variadic number of input spans.
	/// 
	/// \param first  The first span to interleave, there must be at least one of these
	/// \param rest	  The other spans to interleave, there may be any number of spans here. They must all be the exact size of `first.size()`
	/// 
	/// \return A vector of the interleaved spans
	template <typename T, typename... Spans>
		requires (std::same_as<Spans, std::span<const T>> && ...)
	std::vector<T> interleave_alloc(const std::span<const T> first, const Spans... rest)
	{
		PSAPI_PROFILE_FUNCTION();
		std::vector<T> buffer(first.size() * (sizeof...(rest) + 1));
		interleave(std::span<T>(buffer), first, rest...);

		return buffer;
	}

	/// Interleave the spans into a unified buffer taking any number of input spans (could be channels).
	///
	/// could be called like this for example:
	/// 
	/// \code{.cpp}
	/// std::vector<std::span<T>> channels(channel_r, channel_g, channel_b, channel_a);
	/// 
	/// std::vector<T> interleaved = Render::interleave(channels);
	/// \endcode
	/// 
	/// \tparam T The data type to interleave
	/// 
	/// \param spans The input spans to interleave. Must all be the same size
	/// 
	/// \return A vector of the interleaved spans
	template <typename T>
	std::vector<T> interleave_alloc(const std::vector<std::span<const T>> spans)
	{
		if (spans.empty())
		{
			throw std::invalid_argument("Interleave: No spans provided for interleaving.");
		}

		return std::apply(
			[](const auto&... spans) { return interleave_alloc(spans...); },
			Impl::spans_to_tuple(spans)
		);
	}


	/// Interleave the spans into a unified buffer taking any number of input spans (could be channels).
	///
	/// could be called like this for example:
	/// 
	/// \code{.cpp}
	/// std::span<T> interleaved;
	/// std::vector<std::span<T>> channels(channel_r, channel_g, channel_b, channel_a);
	/// 
	/// Render::interleave(interleaved, channels);
	/// \endcode
	/// 
	/// \tparam T The data type to interleave
	/// 
	/// \param buffer The preallocated buffer to interleave into. Must be exactly the size of spans[0].size * spans.size()
	/// \param spans  The input spans to interleave. Must all be the same size
	template <typename T>
	void interleave(std::span<T> buffer, const std::vector<std::span<const T>>& spans)
	{
		if (spans.empty())
		{
			throw std::invalid_argument("Interleave: No spans provided for interleaving.");
		}

		// Forward spans as variadic arguments to the existing interleave function
		std::apply(
			[&](const auto&... spans) { interleave(buffer, spans...); },
			Impl::spans_to_tuple(spans)
		);
	}


} // namespace Render

PSAPI_NAMESPACE_END