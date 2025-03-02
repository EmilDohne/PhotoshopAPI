#include "Macros.h"

#include "Renderer/Renderer.h"

#include <vulkan/vulkan.h>
#include <type_traits>

PSAPI_NAMESPACE_BEGIN

template <typename T>
	requires std::is_same_v<T, bpp8_t> || std::is_same_v<T, bpp16_t> || std::is_same_v<T, bpp32_t>
class VulkanBackend final : public RendererBackend<T>
{
	void init();
};

PSAPI_NAMESPACE_END