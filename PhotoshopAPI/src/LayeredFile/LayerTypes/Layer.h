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

	Layer(const LayerRecord& layerRecord, const ChannelImageData& channelImageData);
};

PSAPI_NAMESPACE_END