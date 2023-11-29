#include "Layer.h"

#include "Macros.h"
#include "Enum.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"
#include "PhotoshopFile/AdditionalLayerInfo.h"
#include "Struct/TaggedBlock.h"

#include <vector>
#include <optional>
#include <string>

PSAPI_NAMESPACE_BEGIN

// Instantiate the template types for Layer
template struct Layer<uint8_t>;
template struct Layer<uint16_t>;
template struct Layer<float32_t>;

template <typename T>
Layer<T>::Layer(const LayerRecord& layerRecord, const ChannelImageData& channelImageData)
{
	m_LayerName = layerRecord.m_LayerName.m_String;
	// To parse the blend mode we must actually check for the presence of the sectionDivider blendMode as this overrides the layerRecord
	// blendmode if it is present
	{
		auto& additionalLayerInfo = layerRecord.m_AdditionalLayerInfo.value();
		auto sectionDivider = additionalLayerInfo.getTaggedBlock<LrSectionTaggedBlock>(Enum::TaggedBlockKey::lrSectionDivider);
		if (sectionDivider.has_value() && sectionDivider.value()->m_BlendMode.has_value())
		{
			m_BlendMode = sectionDivider.value()->m_BlendMode.value();
		}
		else
		{
			m_BlendMode = layerRecord.m_BlendMode;
		}
	}
	m_Opacity = layerRecord.m_Opacity;
	m_Width = static_cast<uint64_t>(layerRecord.m_Right) - layerRecord.m_Left;
	m_Height = static_cast<uint64_t>(layerRecord.m_Bottom) - layerRecord.m_Top;
}




PSAPI_NAMESPACE_END