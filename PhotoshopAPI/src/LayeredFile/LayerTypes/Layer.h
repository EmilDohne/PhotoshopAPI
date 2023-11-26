#pragma once

#include "Macros.h"
#include "Enum.h"
#include "PhotoshopFile/LayerAndMaskInformation.h"

#include <vector>
#include <string>

PSAPI_NAMESPACE_BEGIN

/// Base Struct for Layers of all types (Group, Image, [Adjustment], etc.)
/// Includes the bare minimum 
template <typename T>
struct Layer
{
	std::string m_LayerName;
	std::vector<T> m_LayerMask;
	Enum::BlendMode m_BlendMode;

	uint8_t m_Opacity;	// 0 - 255 despite the appearance being 0-100 in photoshop

	uint64_t m_Width;
	uint64_t m_Height;

	Layer(const LayerRecord& layerRecord, const std::shared_ptr<ChannelImageData<T>> channelImageData)
	{
		m_LayerName = layerRecord.m_LayerName.m_String;
		// To parse the blend mode we must actually check for the presence of the sectionDivider blendMode as this overrides the layerRecord
		// blendmode if it is present
		{
			auto& additionalLayerInfo = layerRecord.m_AdditionalLayerInfo.value();
			auto sectionDivider = additionalLayerInfo.getTaggedBlock<TaggedBlock::LayerSectionDivider>(Enum::TaggedBlockKey::lrSectionDivider);
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
};

PSAPI_NAMESPACE_END