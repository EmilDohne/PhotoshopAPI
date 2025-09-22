#pragma once

#include "Macros.h"

#include <type_traits>
#include <concepts>

#include <OpenImageIO/Imath.h>

PSAPI_NAMESPACE_BEGIN


namespace composite
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

		/// concept for a compositing kernel taking the canvas value and the layer value.
		template <typename _Precision, typename KernelFunc>
			requires precision<_Precision>
		concept kernel = std::is_invocable_r_v<_Precision, KernelFunc, _Precision, _Precision>;

	} // namespace concepts



} // namespace composite

PSAPI_NAMESPACE_END