#include "Layer.h"
#include "ImageLayer.h"
#include "GroupLayer.h"
#include "SectionDividerLayer.h"
#include "TextLayer.h"
#include "SmartObjectLayer.h"
#include "AdjustmentLayer.h"
#include "ArtboardLayer.h"
#include "ShapeLayer.h"

#include "MaskDataMixin.h"
#include "ImageDataMixins.h"

#include "Macros.h"

// To aid in compilation speeds we instantiate the template definitions for all the bitdepth types (since those are what we will actually use).
// These are all defined as externs in the relevant files.

PSAPI_NAMESPACE_BEGIN

template struct Layer<bpp8_t>;
template struct Layer<bpp16_t>;
template struct Layer<bpp32_t>;

template struct GroupLayer<bpp8_t>;
template struct GroupLayer<bpp16_t>;
template struct GroupLayer<bpp32_t>;

template struct ImageLayer<bpp8_t>;
template struct ImageLayer<bpp16_t>;
template struct ImageLayer<bpp32_t>;

template struct SectionDividerLayer<bpp8_t>;
template struct SectionDividerLayer<bpp16_t>;
template struct SectionDividerLayer<bpp32_t>;

template struct ArtboardLayer<bpp8_t>;
template struct ArtboardLayer<bpp16_t>;
template struct ArtboardLayer<bpp32_t>;

template struct AdjustmentLayer<bpp8_t>;
template struct AdjustmentLayer<bpp16_t>;
template struct AdjustmentLayer<bpp32_t>;

template struct ShapeLayer<bpp8_t>;
template struct ShapeLayer<bpp16_t>;
template struct ShapeLayer<bpp32_t>;

template struct SmartObjectLayer<bpp8_t>;
template struct SmartObjectLayer<bpp16_t>;
template struct SmartObjectLayer<bpp32_t>;

template struct TextLayer<bpp8_t>;
template struct TextLayer<bpp16_t>;
template struct TextLayer<bpp32_t>;

template struct MaskMixin<bpp8_t>;
template struct MaskMixin<bpp16_t>;
template struct MaskMixin<bpp32_t>;

template struct ImageDataMixin<bpp8_t>;
template struct ImageDataMixin<bpp16_t>;
template struct ImageDataMixin<bpp32_t>;

template struct WritableImageDataMixin<bpp8_t>;
template struct WritableImageDataMixin<bpp16_t>;
template struct WritableImageDataMixin<bpp32_t>;

PSAPI_NAMESPACE_END