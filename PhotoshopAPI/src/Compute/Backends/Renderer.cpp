#include "Renderer.h"
#include "Logger.h"

#include "GenericCPU/GenericCPURenderer.h"
#include "Vulkan/VulkanRenderer.h"

PSAPI_NAMESPACE_BEGIN

template <typename T>
std::shared_ptr<RendererBackend<T>> RendererBackend<T>::create(Enum::RendererBackendType type)
{
	if (type == Enum::RendererBackendType::Vulkan)
	{
		auto backend = std::make_shared<VulkanBackend<T>>();
		backend->init();
		return backend;
	}
	auto backend = std::make_shared<GenericCPUBacked<T>>();
	backend->init();
	return backend;
}


PSAPI_NAMESPACE_END