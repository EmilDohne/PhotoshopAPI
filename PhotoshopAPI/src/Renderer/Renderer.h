#pragma once

#include "Macros.h"

#include "Core/Render/Render.h"

#include <memory>
#include <type_traits>

PSAPI_NAMESPACE_BEGIN

namespace Enum
{
	/// Enumeration describing the rendering backend to use
	enum class RendererBackendType
	{
		/// Generic CPU "Renderer" this is the default or fallback in case the other rendering
		/// backend does not support it.
		GenericCPU,
		/// Vulkan GPU backend
		VulkanGPU
	};
}


template <typename T>
	requires std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, float32_t>
class RendererBackend
{
public:

	/// Backend initialization and setup, this is backend dependent
	virtual void init() = 0;

	/// Render the image onto the buffer using the given quad mesh. Supersamples the rendering at a 4x4 resolution to ensure anti-aliased edges.
	virtual void render_quad_mesh(Render::ChannelBuffer<T> buffer, Render::ConstChannelBuffer<T> image, const Geometry::QuadMesh<double>& warp_mesh) const = 0;

	/// Create a backend for the given type
	static std::shared_ptr<RendererBackend> create(Enum::RendererBackendType type);

	virtual ~RendererBackend() = default;
};

PSAPI_NAMESPACE_END