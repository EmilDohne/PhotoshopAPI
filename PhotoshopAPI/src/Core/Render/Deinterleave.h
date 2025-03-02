#pragma once

#include "Macros.h"
#include "Util/Logger.h"

#include <vector>
#include <algorithm>
#include <execution>

#include <span>

PSAPI_NAMESPACE_BEGIN

namespace Render
{


	/// Deinterleave a unified buffer into separate spans (channels).
	///
	/// Could be called like this, for example:
	///
	/// \code{.cpp}
	/// std::span<T> channel_r;
	/// std::span<T> channel_g;
	/// std::span<T> channel_b;
	/// std::span<T> channel_a;
	///
	/// std::vector<T> interleaved = ...; // Previously interleaved data
	/// Render::deinterleave(interleaved, channel_r, channel_g, channel_b, channel_a);
	/// \endcode
	///
	/// \tparam T The data type to deinterleave
	/// \tparam ...Spans Variadic number of output spans.
	///
	/// \param interleaved The input buffer containing the interleaved data.
	/// \param deinterleaved The output spans to store the deinterleaved data.
	///                      Must be exactly the size of `interleaved.size() / num_channels`.
	template <typename T, typename... Spans>
	void deinterleave(std::span<const T> interleaved, const Spans... deinterleaved)
	{
		PSAPI_PROFILE_FUNCTION();
		// Collect all spans into an array for iteration
		std::array<std::span<T>, sizeof...(deinterleaved)> spans{ deinterleaved... };

		// Ensure all spans have the same size
		std::size_t size = spans.front().size();
		if (!std::all_of(spans.begin(), spans.end(), [size](const std::span<T>& span) { return span.size() == size; }))
		{
			PSAPI_LOG_ERROR("Deinterleave", "All output spans must have the same size.");
			return;
		}

		// Ensure the interleaved buffer size matches the total size of the spans
		if (interleaved.size() != size * spans.size())
		{
			PSAPI_LOG_ERROR("Deinterleave", "Input buffer size does not match the expected size for deinterleaving.");
			return;
		}

		// Perform deinterleaving
		const std::size_t spans_size = spans.size();
		auto indices = std::views::iota(static_cast<std::size_t>(0), spans.front().size());
		std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [&](auto idx)
			{
				const std::size_t interleaved_base_idx = idx * spans_size;
				for (std::size_t i = 0; i < spans_size; ++i)
				{
					spans[i][idx] = interleaved[interleaved_base_idx + i];
				}
			});
	}


	/// Deinterleave a unified buffer into multiple channels, using the preallocated buffers.
	///
	/// Could be called like this, for example:
	///
	/// \code{.cpp}
	///
	/// std::vector<T> interleaved = ...; // Previously interleaved data
	/// std::vector<std::span<T>> channels = ...; // The channels to deinterleave into
	/// auto deinterleaved = Render::deinterleave(interleaved, channels);
	/// 
	/// \endcode
	/// 
	/// \tparam T The data type to deinterleave.
	///
	/// \param interleaved The input buffer containing the interleaved data.
	/// \param channel_spans The spans to deinterleave into.
	///
	template <typename T>
	void deinterleave(std::span<const T> interleaved, std::vector<std::span<T>>& channel_spans)
	{
		PSAPI_PROFILE_FUNCTION();
		// Ensure all spans have the same size
		std::size_t size = channel_spans.front().size();
		if (!std::all_of(channel_spans.begin(), channel_spans.end(), [size](const std::span<T>& span) { return span.size() == size; }))
		{
			PSAPI_LOG_ERROR("Deinterleave", "All output spans must have the same size.");
			return;
		}

		// Ensure the interleaved buffer size matches the total size of the spans
		if (interleaved.size() != size * channel_spans.size())
		{
			PSAPI_LOG_ERROR("Deinterleave", "Input buffer size does not match the expected size for deinterleaving.");
			return;
		}

		// Perform deinterleaving
		const std::size_t spans_size = channel_spans.size();
		auto indices = std::views::iota(static_cast<std::size_t>(0), channel_spans.front().size());
		std::for_each(std::execution::par_unseq, indices.begin(), indices.end(), [&](auto idx)
			{
				const std::size_t interleaved_base_idx = idx * spans_size;
				for (std::size_t i = 0; i < spans_size; ++i)
				{
					channel_spans[i][idx] = interleaved[interleaved_base_idx + i];
				}
			});
	}


	/// Deinterleave a unified buffer into multiple channels, allocating the output buffers.
	///
	/// 
	/// Could be called like this, for example:
	///
	/// \code{.cpp}
	///
	/// std::vector<T> interleaved = ...; // Previously interleaved data
	/// auto deinterleaved = Render::deinterleave(interleaved, 4); // interleaved stores e.g. 4 channels
	/// 
	/// \endcode
	/// 
	/// \tparam T The data type to deinterleave.
	///
	/// \param interleaved The input buffer containing the interleaved data.
	/// \param num_channels The number of channels to deinterleave into, must match the size the number of channels the interleaved buffer was created with.
	///
	/// \return A vector of vectors, where each vector is a channel of deinterleaved data.
	template <typename T>
	std::vector<std::vector<T>> deinterleave_alloc(std::span<const T> interleaved, std::size_t num_channels)
	{
		PSAPI_PROFILE_FUNCTION();
		// Ensure the input size is divisible by the number of channels
		if (interleaved.size() % num_channels != 0)
		{
			PSAPI_LOG_ERROR("Deinterleave", "Input buffer size must be divisible by the number of channels.");
			return {};
		}

		std::size_t channel_size = interleaved.size() / num_channels;

		// Allocate vectors for each channel
		std::vector<std::vector<T>> channels(num_channels);
		std::for_each(std::execution::par_unseq, channels.begin(), channels.end(), [channel_size](std::vector<T>& channel)
			{
				channel = std::vector<T>(channel_size);
			});

		// Create spans from these vectors and call the base implementation
		std::vector<std::span<T>> channel_spans;
		channel_spans.reserve(num_channels);
		for (auto& channel : channels)
		{
			channel_spans.emplace_back(channel);
		}

		// Call base implementation
		deinterleave(interleaved, channel_spans);

		return channels;
	}


} // namespace Render

PSAPI_NAMESPACE_END