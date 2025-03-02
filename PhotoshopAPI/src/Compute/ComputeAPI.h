#pragma once

#include "Macros.h"

#include "Core/Render/Render.h"

#include <memory>
#include <type_traits>

PSAPI_NAMESPACE_BEGIN

namespace Enum
{
	enum class ComputeDevice
	{
		CPU,
		GPU
	};
}

struct ComputeAPI
{
    inline static void set_device(Enum::ComputeDevice device)
    {
        s_Device = device;
    }

    inline static Enum::ComputeDevice get_device()
    {
        return s_Device;
    }

    template <typename T>
    inline static std::shared_ptr<RendererBackend<T>> get_renderer()
    {
        if (s_Device == Enum::ComputeDevice::CPU)
        {
            return RendererBackend<T>::create(Enum::RendererBackendType::GenericCPU);
        }
        return RendererBackend<T>::create(Enum::RendererBackendType::VulkanGPU);
    }

private:
    static Enum::ComputeDevice s_Device;
};

// Define the static variable
Enum::ComputeDevice ComputeAPI::s_Device = Enum::ComputeDevice::CPU;

PSAPI_NAMESPACE_END