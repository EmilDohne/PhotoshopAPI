#include "Macros.h"

#include "Compute/Backends/Renderer.h"
#include "VkPrimitives.h"

#include <type_traits>
#include <memory>

PSAPI_NAMESPACE_BEGIN

template <typename T>
	requires std::is_same_v<T, bpp8_t> || std::is_same_v<T, bpp16_t> || std::is_same_v<T, bpp32_t>
class VulkanBackend final : public RendererBackend<T>
{

	void init() override;

private:
	std::unique_ptr<Vulkan::BaseData> m_VulkanData = nullptr;
};

PSAPI_NAMESPACE_END