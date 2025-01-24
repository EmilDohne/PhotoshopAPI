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
		/// Very similar to std::is_floating_point_v but additionally checking it is Imath::half as we want to support both
		/// half, float and double etc.
		template <typename _Precision>
		constexpr bool is_floating_v = std::is_floating_point_v<_Precision> || std::is_same_v<_Precision, Imath::half>;

		/// concept requiring the given _Precision template arg to fulfill concepts::is_floating_v
		template <typename _Precision>
		concept precision = is_floating_v<_Precision>;

		/// concept for a compositing kernel taking the canvas value, the layer value as well as the layer alpha.
		template <typename T, typename KernelFunc>
		concept kernel = std::is_invocable_r_v<T, KernelFunc, T, T, T, T>;
	}


	/// Processing kernels for the different blend modes, operate on one pixel at a time.
	namespace kernel
	{
		namespace impl
		{
			/// Compute the maximum T value for a given template arg, for integral types this would return std::numeric_limits<T>::max
			/// while floating point values return 1.0f
			template <typename T>
			constexpr T calc_max_t()
			{
				constexpr T max_t = std::numeric_limits<T>::max();
				if constexpr (concepts::is_floating_v<T>)
				{
					max_t = static_cast<T>(1.0f);
				}
				return max_t;
			}


			/// Apply a round and clamp of the given value if template argument T is integral. While the value is passed as 
			/// `_Precision` the actual type of the value should be represented by T. Returns either the rounded and clamped value
			/// or in the case of the value being floating point it simply returns the same value again which would lead to a no-op
			template <typename T, typename _Precision>
				requires concepts::precision<_Precision>
			inline _Precision round_and_clamp_integral(_Precision value)
			{
				if constexpr (std::is_integral_v<T>)
				{
					value = std::round(value);
					return std::clamp(
						value,
						static_cast<_Precision>(std::numeric_limits<T>::min()),
						static_cast<_Precision>(std::numeric_limits<T>::max())
					);
				}
				return value;
			}
		}


		template <typename T, typename _Precision = float> 
			requires concepts::precision<_Precision>
		inline T normal(T _canvas_pixel, T _canvas_alpha, T _layer_pixel, T _layer_alpha)
		{
			//return _layer_pixel;

			// Convert input values to the specified precision type
			_Precision canvas	= static_cast<_Precision>(_canvas_pixel) / impl::calc_max_t<T>();
			_Precision c_alpha  = static_cast<_Precision>(_canvas_alpha) / impl::calc_max_t<T>();
			_Precision layer	= static_cast<_Precision>(_layer_pixel)  / impl::calc_max_t<T>();
			_Precision l_alpha	= static_cast<_Precision>(_layer_alpha)  / impl::calc_max_t<T>();

			// The actual blending operation.
			//_Precision result = (layer * l_alpha) + (canvas * c_alpha * (static_cast<_Precision>(1) - l_alpha));
			_Precision result = layer + canvas * (static_cast<_Precision>(1) - l_alpha);

			result = result * impl::calc_max_t<T>();
			result = impl::round_and_clamp_integral<T, _Precision>(result);
			return static_cast<T>(result);
		}

		/// Generic alpha compositing kernel
		template <typename T, typename _Precision = float>
			requires concepts::precision<_Precision>
		inline T alpha(T canvas_alpha, T layer_alpha)
		{
			_Precision _c_alpha = static_cast<_Precision>(canvas_alpha) / impl::calc_max_t<T>();
			_Precision _l_alpha = static_cast<_Precision>(layer_alpha)  / impl::calc_max_t<T>();

			_Precision result = _l_alpha + _c_alpha * (1.0f - _l_alpha);
			
			if constexpr (concepts::is_floating_v<T>)
			{
				return static_cast<T>(result);
			}
			else
			{
				result = result * impl::calc_max_t<T>();
				result = impl::round_and_clamp_integral<T, _Precision>(result);
				return static_cast<T>(result);
			}
		}

	}


	/// Implementation details of the compositing logic dealing with iterating the images and applying the kernels.
	namespace impl
	{
		namespace rgb
		{

			/// Template function to iterate and apply an image layering kernel (one for each blendmode) for RGB channels.
			/// this works on the principle that the compositing for RGB is separable unlike for e.g. CMYK which needs to be
			/// handled differently.
			/// 
			/// This takes care of a couple of things:
			/// - Compose the two layers on top of one another using the computed alpha (from alpha, mask and opacity)
			/// - Compose the alpha channels of these two together 
			/// - Iterate only what is necessary by taking the intersection of the canvas bbox and the layer bbox.
			/// 
			/// Compositing only happens for any channels that are present both on the canvas and layer but for all practical
			/// purposes it can be assumed that both the canvas and layer will hold at least the same color components with 
			/// the main differences being the presence of alpha channels and/or masks.
			/// If the canvas holds no alpha we generate a new one and modify it in-place. The alpha will then be updated according
			/// to the computation.
			/// 
			/// \tparam T 
			///		The bit-depth of the image data
			/// \tparam _Precision 
			///		The precision at which to perform the computation (for integral types). If passing floating point image it would
			///		be be best to have `_Precision` be the same as `T` as this would make a lot of the conversion a no-op.
			/// \tparam KernelFunc
			///		The kernel function to apply to the image, must satisfy `concepts::kernel`. I.e. the kernel must take a canvas pixel
			///		value, a layer pixel value and the layer alpha. see `kernel::normal` for an example.
			/// 
			/// \param canvas 
			///		The canvas onto which to draw the layer with the given kernel.
			/// \param layer
			///		The layer to compose on top of the canvas
			/// \param kernel
			///		The kernel to apply to the color channels of the image (alpha is handled separately)
			/// 
			/// \throws std::invalid_argument If the canvas has a mask channel.
			/// \throws std::invalid_argument If the canvas has a non-one opacity.
			template <typename T, typename _Precision, typename KernelFunc>
				requires concepts::precision<_Precision> && concepts::kernel<T, KernelFunc>
			void iter_apply(Render::ImageBuffer<T>& canvas, Render::ConstImageBuffer<T>& layer, KernelFunc kernel_func)
			{
				if (canvas.channels.size() == 0 || layer.channels.size() == 0)
				{
					PSAPI_LOG_DEBUG("Composite", 
						"Skipping compositing of layer '%s' as either the layer or the canvas has no channels. Canvas channels: %zu; Layer channels: %zu", 
						layer.metadata.name, canvas.channels.size(), layer.channels.size());
					return;
				}

				if (canvas.has_mask())
				{
					throw std::invalid_argument("Unable to composite layers if the canvas has a mask channel as this is not valid");
				}
				if (canvas.metadata.opacity != 1.0f)
				{
					throw std::invalid_argument("Unable to composite layers if the canvas has an opacity that isnt 1.0f as this is not valid");
				}

				// Compute the intersection of the canvas and the layer as the layer may go outside of the canvas' bbox.
				auto canvas_bbox = Geometry::BoundingBox<int>(Geometry::Point2D<int>(0, 0), Geometry::Point2D<int>(canvas.width, canvas.height));
				auto layer_bbox = layer.bbox(canvas.width, canvas.height);
				auto _intersected_bbox = Geometry::BoundingBox<int>::intersect(canvas_bbox, layer_bbox);
				if (!_intersected_bbox)
				{
					PSAPI_LOG_DEBUG("Composite", "Skipping compositing of layer '%s' as the intersected bbox is 0-sized.", layer.metadata.name);
					return;
				}
				auto intersected_bbox = _intersected_bbox.value();

				// Limit the computation to the region of interest (ROI) of the layer to avoid iterating channels outside of the 
				size_t min_y = std::max<size_t>(static_cast<size_t>(std::round(intersected_bbox.minimum.y)), 0);
				size_t max_y = std::min<size_t>(static_cast<size_t>(std::round(intersected_bbox.maximum.y)), canvas.height - 1);
				size_t min_x = std::max<size_t>(static_cast<size_t>(std::round(intersected_bbox.minimum.x)), 0);
				size_t max_x = std::min<size_t>(static_cast<size_t>(std::round(intersected_bbox.maximum.x)), canvas.width - 1);

				auto vertical_iter = std::views::iota(min_y, max_y);
				auto horizontal_iter = std::views::iota(min_x, max_x);

				auto canvas_alpha = canvas.compute_alpha<_Precision>(canvas.width, canvas.height);
				auto layer_alpha = layer.compute_alpha<_Precision>(canvas.width, canvas.height);

				for (auto [_index, canvas_channel] : canvas.channels)
				{
					// Since we tackle alpha separately we want to ignore it here
					if (_index == -1)
					{
						continue;
					}
					// Some layers may not have all channels present, if that is the case we simply skip it
					if (!layer.channels.contains(_index))
					{
						continue;
					}
					
					const auto layer_channel = layer.channels.at(_index);

					// Iterate the channels and apply the kernel.
					std::for_each(std::execution::par_unseq, vertical_iter.begin(), vertical_iter.end(), [&](size_t y)
						{
							for (auto x : horizontal_iter)
							{
								Geometry::Point2D<int> _point = { static_cast<int>(x), static_cast<int>(y) };
								const auto point_canvas = _point - canvas_bbox.minimum;
								const auto point_layer = _point - layer_bbox.minimum;

								auto idx_canvas = canvas_channel.index(point_canvas.x, point_canvas.y);
								auto idx_layer = layer_channel.index(point_layer.x, point_layer.y);

								// Apply the kernel
								canvas_channel.buffer[idx_canvas] = kernel_func(
									canvas_channel.pixel(point_canvas.x, point_canvas.y), 
									canvas_alpha[idx_canvas],
									layer_channel.pixel(point_layer.x, point_layer.y),
									layer_alpha[idx_layer]
									);
							}
						});
				}
				
				{
					// since all of these are the same size we can just grab any channel for our index calculations
					const auto& [_ci, _canvas_channel] = *canvas.channels.begin();
					const auto& [_li, _layer_channel] = *layer.channels.begin();

					if (canvas_alpha.size() < _canvas_channel.size())
					{
						throw std::runtime_error("Internal error: canvas alpha is larger than canvas channels");
					}
					if (layer_alpha.size() < _layer_channel.size())
					{
						throw std::runtime_error("Internal error: layer alpha is larger than layer channels");
					}

					// Apply the alpha compositing as the last step.
					std::for_each(std::execution::par_unseq, vertical_iter.begin(), vertical_iter.end(), [&](size_t y)
						{
							for (auto x : horizontal_iter)
							{
								Geometry::Point2D<int> _point = { static_cast<int>(x), static_cast<int>(y) };
								const auto point_canvas = _point - canvas_bbox.minimum;
								const auto point_layer = _point - layer_bbox.minimum;

								auto idx_canvas = _canvas_channel.index(point_canvas.x, point_canvas.y);
								auto idx_layer = _layer_channel.index(point_layer.x, point_layer.y);

								// Apply the kernel and compute alpha and pixel values
								T alpha = kernel::alpha(canvas_alpha[idx_canvas], layer_alpha[idx_layer]);
								canvas_alpha[idx_canvas] = alpha;
							}
						});

				}				

				// We do not want to clear the alpha of the canvas since we are modifying it in-place
				// allowing the next iteration to just have canvas.compute_alpha return a view over it.
				layer.clear_cached_alpha();
			}

		}
	}


	/// Composite a layer over the canvas using the given blendmode (not all yet supported).
	/// in this context the canvas is nothing special and could just be another layer but in most cases
	/// we will be compositing down to the photoshop canvas itself.
	/// 
	/// \tparam T 
	///		The bit-depth of the image data
	/// \tparam _Precision 
	///		The precision at which to perform the computation (for integral types). If passing floating point image it would
	///		be be best to have `_Precision` be the same as `T` as this would make a lot of the conversion a no-op.
	/// 
	/// \param canvas 
	///		The canvas onto which to draw the layer with the given blendmode.
	/// \param layer
	///		The layer to compose on top of the canvas
	/// \param blend_mode
	///		The blend mode to apply
	/// 
	/// \throws std::invalid_argument If the canvas has a mask channel.
	/// \throws std::invalid_argument If the canvas has a non-one opacity.
	/// \throws std::runtime_error	  If the given blendmode is not yet implemented
	template <typename T, typename _Precision = float>
		requires concepts::precision<_Precision>
	void composite_rgb(Render::ImageBuffer<T>& canvas, Render::ConstImageBuffer<T>& layer, Enum::BlendMode blend_mode)
	{
		if (blend_mode == Enum::BlendMode::Normal)
		{
			impl::rgb::iter_apply<T, _Precision>(canvas, layer, kernel::normal<T, _Precision>);
		}
		else
		{
			std::string blend_mode_str = Enum::getBlendMode<Enum::BlendMode, std::string>(blend_mode).value_or("Unknown");
			throw std::runtime_error(fmt::format("blendmode {} is not yet implemented for compositing", blend_mode_str));
		}
	}
}

PSAPI_NAMESPACE_END