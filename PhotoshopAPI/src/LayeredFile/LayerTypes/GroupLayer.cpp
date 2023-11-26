#include "GroupLayer.h"

PSAPI_NAMESPACE_BEGIN


// Instantiate the template types for GroupLayer
template struct GroupLayer<uint8_t>;
template struct GroupLayer<uint16_t>;
template struct GroupLayer<float32_t>;


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
GroupLayer<T>::GroupLayer(const LayerRecord& layerRecord, const std::shared_ptr<ChannelImageData<T>> channelImageData) : Layer<T>(layerRecord, channelImageData)
{
}


PSAPI_NAMESPACE_END