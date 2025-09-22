#pragma once

#include "Macros.h"

#include "LayeredFile/concepts.h"
#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/GroupLayer.h"

#include "_core.h"
#include "concepts.h"

PSAPI_NAMESPACE_BEGIN


namespace composite
{

	namespace impl
	{

		std::vector<T> composite_scope()

	}


	template <typename T>
		requires concepts::bit_depth<T>
	std::vector<T> composite(std::variant<GroupLayer<T>, LayeredFile<T>>& source, Enum::ColorMode color_mode)
	{

	}
	

} // namespace composite

PSAPI_NAMESPACE_END