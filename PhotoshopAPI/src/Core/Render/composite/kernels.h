/*
Header containing the atomic kernels operating on a single pixel at a time that compute the composite of a given 
blend mode. Available blend modes are:

- normal
- multiply
- screen
- overlay
- darken
- lighten
- color dodge
- color burn
- linear dodge
- linear burn
- hard light
- soft light
- vivid light
- linear light
- pin light
- difference
- exclusion
- subtract
- hard mix
- divide

*/

#pragma once

#include "Macros.h"

#include <type_traits>
#include <concepts>
#include <cmath>
#include <ranges>

#include "concepts.h"

PSAPI_NAMESPACE_BEGIN


namespace composite
{

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

				static_assert(max_t != 0);
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

		/// \brief Wrap the given kernel func to convert from the image depth to the working depth as well as converting
		/// 
		/// \tparam T 
		/// \tparam _Precision 
		/// \tparam KernelFunc 
		/// \param _canvas_pixel 
		/// \param _canvas_alpha 
		/// \param _layer_pixel 
		/// \param _layer_alpha 
		/// \param func 
		/// \return 
		template <typename T, typename KernelFunc, typename _Precision = float> 
			requires concepts::precision<_Precision> && concepts::kernel<_Precision, KernelFunc>
		inline T wrap_kernel(T _canvas_pixel, T _canvas_alpha, T _layer_pixel, T _layer_alpha, KernelFunc func)
		{
			// Convert input values to the specified precision type
			_Precision canvas	= static_cast<_Precision>(_canvas_pixel) / impl::calc_max_t<T>();
			_Precision c_alpha  = static_cast<_Precision>(_canvas_alpha) / impl::calc_max_t<T>();
			_Precision layer	= static_cast<_Precision>(_layer_pixel)  / impl::calc_max_t<T>();
			_Precision l_alpha	= static_cast<_Precision>(_layer_alpha)  / impl::calc_max_t<T>();

			// The actual blending operation.
			_Precision pixel_result = func(canvas, layer);
			// Apply the alpha to blend the resulting pixel value with the underlying canvas.
			_Precision result = layer + canvas * (static_cast<_Precision>(1) - l_alpha);

			// Clamp and cast back to the pixel value.
			result = result * impl::calc_max_t<T>();
			result = impl::round_and_clamp_integral<T, _Precision>(result);
			return static_cast<T>(result);
		}

		template <typename _Precision = float> 
			requires concepts::precision<_Precision>
		inline _Precision normal(_Precision canvas, _Precision layer)
		{
			return layer;
		}

		template <typename _Precision = float> 
			requires concepts::precision<_Precision>
		inline _Precision multiply(_Precision canvas, _Precision layer)
		{
			return canvas * layer;
		}

		template <typename _Precision = float> 
			requires concepts::precision<_Precision>
		inline _Precision screen(_Precision canvas, _Precision layer)
		{
			return canvas + layer - (canvas * layer);
		}


		template <typename _Precision = float> 
			requires concepts::precision<_Precision>
		inline _Precision hard_light(_Precision canvas, _Precision layer)
		{
			// Hard light branches depending on the pixel value. If it is above 50% then we screen it, otherwise
			// we multiply.
			if (layer <= static_cast<_Precision>(0.5))
			{
				return multiply(canvas, static_cast<_Precision>(2) * layer);
			}
			else
			{
				return screen(canvas, static_cast<_Precision>(2) * layer - static_cast<_Precision>(1));
			}	
		}

		template <typename _Precision = float> 
			requires concepts::precision<_Precision>
		inline _Precision overlay(_Precision canvas, _Precision layer)
		{
			return hard_light(canvas, layer);
		}

		template <typename _Precision = float> 
			requires concepts::precision<_Precision>
		inline _Precision darken(_Precision canvas, _Precision layer)
		{
			return std::fmin(canvas, layer);
		}

		template <typename _Precision = float> 
			requires concepts::precision<_Precision>
		inline _Precision lighten(_Precision canvas, _Precision layer)
		{
			return std::fmax(canvas, layer);
		}

		template <typename _Precision = float> 
			requires concepts::precision<_Precision>
		inline _Precision color_dodge(_Precision canvas, _Precision layer)
		{
			constexpr _Precision eps = static_cast<_Precision>(1e-9);
			constexpr _Precision s = 1.0f;

			if (layer >= static_cast<_Precision>(1))
				return static_cast<_Precision>(1);  // Cs == 1
			if (canvas <= static_cast<_Precision>(0)) 
				return static_cast<_Precision>(0);  // Cb == 0

			_Precision result = canvas / (s * (static_cast<_Precision>(1) - layer + eps));
			return (result > static_cast<_Precision>(1)) ? static_cast<_Precision>(1) : result;
		}

		template <typename _Precision = float> 
			requires concepts::precision<_Precision>
		inline _Precision color_burn(_Precision canvas, _Precision layer)
		{
			constexpr _Precision eps = static_cast<_Precision>(1e-9);

			if (canvas >= static_cast<_Precision>(1)) 
				return static_cast<_Precision>(1);  // canvas == 1
			if (layer <= static_cast<_Precision>(0)) 
				return static_cast<_Precision>(0);  // layer == 0

			_Precision result = static_cast<_Precision>(1) - ((static_cast<_Precision>(1) - canvas) / (s * layer + eps));
			return (result > static_cast<_Precision>(1)) ? static_cast<_Precision>(1) : result;
		}

		template <typename _Precision = float> 
			requires concepts::precision<_Precision>
		inline _Precision linear_dodge(_Precision canvas, _Precision layer)
		{
			return std::fmin(static_cast<_Precision>(1), canvas + layer);
		}

		template <typename _Precision = float> 
			requires concepts::precision<_Precision>
		inline _Precision linear_burn(_Precision canvas, _Precision layer)
		{
			return std::fmax(static_cast<_Precision>(0), canvas + layer - static_cast<_Precision>(1));
		}

		template <typename _Precision = float> 
			requires concepts::precision<_Precision>
		inline _Precision soft_light(_Precision canvas, _Precision layer)
		{
			return std::fmax(static_cast<_Precision>(0), canvas + layer - static_cast<_Precision>(1));
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

	} // namespace kernel


} // namespace composite

PSAPI_NAMESPACE_END