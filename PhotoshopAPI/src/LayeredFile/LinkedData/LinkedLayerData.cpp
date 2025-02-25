#include "LinkedLayerData.h"

#include "Macros.h"

PSAPI_NAMESPACE_BEGIN

template struct LinkedLayerData<bpp8_t>;
template struct LinkedLayerData<bpp16_t>;
template struct LinkedLayerData<bpp32_t>;

template struct LinkedLayers<bpp8_t>;
template struct LinkedLayers<bpp16_t>;
template struct LinkedLayers<bpp32_t>;

PSAPI_NAMESPACE_END