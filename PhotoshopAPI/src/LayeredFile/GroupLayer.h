#pragma once

#include "Macros.h"
#include "Layer.h"
#include "ImageLayer.h"

#include <vector>
#include <memory>


PSAPI_NAMESPACE_BEGIN

template <typename T>
struct GroupLayer : Layer
{
	std::vector<std::shared_ptr<Layer<T>> m_Layers;
};


PSAPI_NAMESPACE_END