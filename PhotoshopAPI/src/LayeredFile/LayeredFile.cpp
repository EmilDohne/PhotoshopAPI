#include "LayeredFile.h"

#include "PhotoshopFile/PhotoshopFile.h"
#include "Macros.h"

#include <vector>


#include <variant>
#include <memory>
#include <filesystem>
#include <algorithm>


PSAPI_NAMESPACE_BEGIN


template struct LayeredFile<bpp8_t>;
template struct LayeredFile<bpp16_t>;
template struct LayeredFile<bpp32_t>;

PSAPI_NAMESPACE_END