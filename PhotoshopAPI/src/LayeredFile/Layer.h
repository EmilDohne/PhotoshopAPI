#pragma once

#include "Macros.h"
#include "Enum.h"

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

	uint8_t m_Opacity;	// Limited to 0-100

	uint64_t m_Width;
	uint64_t m_Height;
};

PSAPI_NAMESPACE_END