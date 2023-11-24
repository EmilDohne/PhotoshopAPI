#include "ImageLayer.h"

#include "Layer.h"


PSAPI_NAMESPACE_BEGIN


// Instantiate the template types for ImageLayer
template struct ImageLayer<uint8_t>;
template struct ImageLayer<uint16_t>;
template struct ImageLayer<float32_t>;


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
ImageLayer<T>::ImageLayer(const LayerRecord& layerRecord)
{
	Layer<T>::m_LayerName = layerRecord.m_LayerName.m_String;
}


PSAPI_NAMESPACE_END