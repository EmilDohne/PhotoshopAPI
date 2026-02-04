#pragma once

#include <type_traits>

#include <OpenColorIO/OpenColorIO.h>

#include <Macros.h>
#include <LayeredFile/concepts.h>

namespace OCIO = OCIO_NAMESPACE;


PSAPI_NAMESPACE_BEGIN


namespace ocio
{

	/// \brief Maps the template argument `T` to a OCIO bitdepth, supported values are: u8, u16, f32
	template <typename T>
		requires concepts::bit_depth<T>
	OCIO::BitDepth map_to_bitdepth()
	{
		using U = std::remove_cv_t<T>; // Strip const/volatile qualifiers

		if constexpr (std::is_floating_point_v<U>)
		{
			return OCIO::BitDepth::BIT_DEPTH_F32;
		}
		
		if constexpr (sizeof(U) == 1)
		{
			return OCIO::BitDepth::BIT_DEPTH_UINT8;
		}
		else if constexpr (sizeof(U) == 2)
		{
			return OCIO::BitDepth::BIT_DEPTH_UINT16;
		}

		throw std::runtime_error("Internal Error: Invalid type override passed to ocio::map_to_bitdepth");
	}

}

PSAPI_NAMESPACE_END