#pragma once

#include "Macros.h"


#include "ImageBuffer.h"

#include <type_traits>
#include <concepts>
#include <ranges>

#include <fmt/format.h>
#include <OpenImageIO/Imath.h>

PSAPI_NAMESPACE_BEGIN


namespace Composite
{
	namespace concepts
	{
		template <typename _Precision>
		concept precision = std::is_floating_point_v<_Precision> || std::is_same_v<_Precision, Imath::half>;

		template <typename T, typename KernelFunc, typename _Precision>
		concept iter_kernel = precision && std::is_invocable_r_v<T, KernelFunc, T, T, T>;
	}

	namespace kernel
	{
		namespace impl
		{
			template <typename T>
			constexpr T calc_max_t()
			{
				constexpr T max_t = std::numeric_limits<T>::max();
				if constexpr (std::is_floating_point_v<T> || std::is_same_v<T, Imath::half>)
				{
					max_t = static_cast<T>(1.0f);
				}
				return max_t;
			}
		}


		template <typename T, typename _Precision = float> 
			requires concepts::precision
		inline T normal(T canvas_pixel, T layer_pixel, T layer_alpha)
		{
			// Convert input values to the specified precision type
			_Precision canvas	= static_cast<_Precision>(canvas_pixel);
			_Precision layer	= static_cast<_Precision>(layer_pixel);
			_Precision l_alpha	= static_cast<_Precision>(layer_alpha);

			// The actual blending operation.
			_Precision result = layer * l_alpha + canvas * (static_cast<_Precision>(1) - l_alpha);

			if constexpr (std::is_integral_v<T>)
			{
				result = std::round(result);
				result = std::clamp(
					result, 
					static_cast<_Precision>(std::numeric_limits<T>::min()),
					static_cast<_Precision>(std::numeric_limits<T>::max())
				);
			}
			return static_cast<T>(result);
		}

		/// Generic alpha compositing kernel
		template <typename T, typename _Precision = float>
			requires concepts::precision
		inline T alpha(T canvas_alpha, T layer_alpha, float layer_opacity)
		{
			_Precision precomposed = static_cast<_Precision>(canvas_alpha) * layer_alpha * layer_opacity;
			if constexpr (std::is_integral_v<T>)
			{
				precomposed = std::round(precomposed);
				precomposed = std::clamp(
					precomposed,
					static_cast<_Precision>(std::numeric_limits<T>::min()),
					static_cast<_Precision>(std::numeric_limits<T>::max())
				);
			}
			return static_cast<T>(precomposed / impl::calc_max_t());
		}

	}


	namespace impl
	{
		template <typename T, typename _Precision = float, typename KernelFunc>
		requires concepts::iter_kernel<T, KernelFunc, _Precision>
		void iter_apply(ImageBuffer<T> canvas, const ConstImageBuffer<T> layer, KernelFunc kernel_func)
		{
			if (canvas.has_mask())
			{
				throw std::invalid_argument("Unable to composite layers if the canvas has a mask channel as this is not valid");
			}

			// Compute the intersection of the canvas and the layer as the layer may go outside of the canvas' bbox.
			auto canvas_bbox = Geometry::BoundingBox<int>(Geometry::Point2D<int>(0, 0), Geometry::Point2D<int>(canvas.width, canvas.height));
			auto layer_bbox = layer.bbox(canvas.width, canvas.height);
			auto intersected_bbox = Geometry::BoundingBox<int>::intersect(canvas_bbox, layer_bbox);
			if (intersected_bbox.width() == 0 && intersected_bbox.height())
			{
				PSAPI_LOG_DEBUG("Composite", "Skipping compositing of layer '%s' as the intersected bbox is 0-sized.", layer.name);
				return;
			}

			// Limit the computation to the region of interest (ROI) of the layer to avoid iterating channels outside of the 
			size_t min_y = std::max<size_t>(static_cast<size_t>(std::round(intersected_bbox.minimum.y)), 0);
			size_t max_y = std::min<size_t>(static_cast<size_t>(std::round(intersected_bbox.maximum.y)), canvas.height - 1);
			size_t min_x = std::max<size_t>(static_cast<size_t>(std::round(intersected_bbox.minimum.x)), 0);
			size_t max_x = std::min<size_t>(static_cast<size_t>(std::round(intersected_bbox.maximum.x)), canvas.width - 1);

			auto vertical_iter = std::views::iota(min_y, max_y);
			auto horizontal_iter = std::views::iota(min_x, max_x);

			auto canvas_alpha = canvas.compute_alpha();
			auto layer_alpha = layer.compute_alpha();

			for (auto [_index, canvas_channel] : canvas.channels)
			{
				// Some layers may not have all channels present, if that is the case we simply skip it
				if (!layer.channels.contains(_index))
				{
					continue;
				}
				const auto layer_channel = layer.channel.at(_index);

				// Iterate the channels and apply the kernel.
				std::for_each(std::execution::par_unseq, vertical_iter.begin(), vertical_iter.end(), [&](size_t y)
					{
						for (auto x : horizontal_iter)
						{
							Geometry::Point2D<size_t> _point = { x, y };
							const auto point_canvas	= _point - canvas_bbox.minimum;
							const auto point_layer	= _point - layer_bbox.minimum;

							auto idx_canvas = canvas.index(point_canvas);
							auto idx_layer = layer.index(point_layer);

							// Apply the kernel
							T value = kernel_func(canvas_channel.pixel(point_canvas), layer_channel.pixel(point_layer), layer_alpha[idx_layer]);
							canvas_channel.buffer[idx_canvas] = value;
						}
					});
			}

			// Apply the alpha compositing as the last step.
			std::for_each(std::execution::par_unseq, vertical_iter.begin(), vertical_iter.end(), [&](size_t y)
				{
					for (auto x : horizontal_iter)
					{
						Geometry::Point2D<size_t> _point = { x, y };
						const auto point_canvas = _point - canvas_bbox.minimum;
						const auto point_layer = _point - layer_bbox.minimum;

						auto idx_canvas = canvas.index(point_canvas);
						auto idx_layer = layer.index(point_layer);

						// Apply the kernel and compute alpha and pixel values
						T alpha = kernel::alpha(canvas_alpha[idx_canvas], layer_alpha[idx_layer], layer.opacity);
						canvas_alpha[idx_canvas] = alpha;
					}
				});
			
		}
	}

	template <typename T, typename _Precision = float>
	requires concepts::precision
	void composite(ImageBuffer<T> canvas, const ConstImageBuffer<T> layer, Enum::BlendMode blend_mode)
	{
		if (blend_mode == Enum::BlendMode::Normal)
		{
			Impl::iter_apply<T, _Precision>(canvas, layer, kernel::normal);
		}
		else
		{
			throw std::runtime_error(fmt::format("blendmode {} is not yet implemented for compositing", Enum::getBlendMode(blend_mode).value_or("Unknown")));
		}
	}
}