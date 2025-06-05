#pragma once

#include "Macros.h"

#include "Core/Geometry/Point.h"
#include "Core/Geometry/Mesh.h"
#include "Core/Render/Render.h"

#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

#define VMA_STATIC_VULKAN_FUNCTION 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vk_mem_alloc.h>

#include <memory>
#include <filesystem>
#include <vector>
#include <span>
#include <ranges>
#include <limits>


PSAPI_NAMESPACE_BEGIN

namespace Vulkan
{
	template <typename T>
	struct RenderQuadMeshData final : public BaseData
	{
		constexpr static uint32_t ce_descriptor_count = 3;
		constexpr static std::string ce_spv_path = "shaders/render_quadmesh.comp";


		RenderQuadMeshData(
			std::unique_ptr<Vulkan::InitInstance> instance,
			Render::ChannelBuffer<T> buffer,
			Render::ConstChannelBuffer<T> image,
			const Geometry::QuadMesh<double>& warp_mesh
		) : BaseData(std::move(instance), Vulkan::RenderQuadMeshData::spv_path, Vulkan::RenderQuadMeshData::descriptor_count)
		{

		};

		
	protected:

		std::array<uint32_t, 3> compute_workgroup_sizes() const override
		{
			std::array<uint32_t, 3> sizes = {};
			sizes[0] = (m_Canvas.width + BaseData::ce_workgroup_sizes[0] - 1) / BaseData::ce_workgroup_sizes[0];
			sizes[1] = (m_Canvas.height + BaseData::ce_workgroup_sizes[1] - 1) / BaseData::ce_workgroup_sizes[1];
			sizes[0] = 1;

			return sizes;
		}


	private:

		Render::ChannelBuffer<T> m_Canvas;
		Render::ConstChannelBuffer<T> m_Image;

	};
}


PSAPI_NAMESPACE_END