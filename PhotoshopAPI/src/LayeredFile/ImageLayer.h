#pragma once

#include "Macros.h"
#include "Enum.h"
#include "Layer.h"

#include <unordered_map>

PSAPI_NAMESPACE_BEGIN

template <typename T>
struct ImageLayer : Layer
{
	// Store the image data as a per-channel map to be used later
	std::unordered_map<Enum::ChannelID, std::vector<T>> m_ImageData;
};

PSAPI_NAMESPACE_END