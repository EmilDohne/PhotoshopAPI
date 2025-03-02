/*
Basic vulkan primitives and their initialization 
*/

#pragma once

#include "Macros.h"

#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
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

    /// Vulkan initialization structure holding the instance, device and all the dispatch tables.
    struct InitInstance
    {
        vkb::Instance instance{};
        vkb::InstanceDispatchTable instance_dispatch{};
        vkb::Device device{};
        vkb::DispatchTable dispatch{};
        VmaAllocator allocator{};

        /// Initialize the vulkan context, will handle initialization of all the Vulkan Bootstrap
        /// primitives as well as the vulkan memory allocator.
        void init();

        bool is_initialized() const noexcept;

        ~InitInstance();

    private:

        /// Setup VMA (Vulkan MemoryAllocator) for the instance storing it globally. Requires all the 
        /// Vulkan bootstrap primitives to have already been initialized!
        void init_vma();

        bool m_Initialized = false;
    };


    /// POD Memory buffer for us to interact with gpu device memory. Construction and cleanup should be handled by 
    /// holders of this structure as the construction and destruction require a vulkan instance.
    struct MemoryBuffer
    {
        VkBuffer buffer = nullptr;
        VmaAllocation allocation = nullptr;
        VmaAllocationInfo allocation_info{};
    };


    /// Base Data structure for a vulkan compute pipeline. Will have to be extended for each specific use case 
    /// to define the individual buffers and device memory etc.
    struct BaseData
    {
        /// X, Y and Z workgroup sizes respectively, this should be set according to the shader
        /// that we are initializing the primitive with.
        std::array<uint32_t, 3> workgroup_sizes = { 1, 1, 1 };

        VkQueue queue = nullptr;

        std::unique_ptr<Vulkan::InitInstance> vk_instance = nullptr;

        VkPipelineLayout pipeline_layout = nullptr;
        VkPipeline compute_pipeline = nullptr;

        VkCommandPool command_pool = nullptr;
        VkCommandBuffer command_buffer = nullptr;

        VkDescriptorSet descriptor_set = nullptr;
        VkDescriptorPool descriptor_pool = nullptr;
        VkDescriptorSetLayout descriptor_set_layout = nullptr;

        /// Mapping of GPU buffers by a string name for easy access. This way you could e.g.
        /// upload a 'canvas' and 'layer' buffer and access these by those names retrieving the 
        /// memmapped data. Since these buffers are allocated using VMA they are automatically
        /// memory mapped for easy cpu access.
        std::unordered_map<std::string, std::unique_ptr<MemoryBuffer>> buffers;

        /// Initialize the base vulkan data structure with the given instance ensuring that:
        ///
        /// - A Vulkan queue is fetched.
        /// - A compute pipeline is created
        /// - The descriptors are allocated and layed out.
        /// - The Command Pool is registered
        /// 
        /// After initialization all of the member variables will point to valid items.
        BaseData(std::unique_ptr<Vulkan::InitInstance> instance, std::filesystem::path spv_path, uint32_t num_descriptors);

        /// Submit the work to the GPU once all the needed buffers have been uploaded to the GPU after which this will block
        /// until the GPU operation is done.
        void create_and_submit();

        // Perform cleanup of all the vulkan instances and primitives.
        virtual ~BaseData();

    protected:

        /// Push and upload a given buffer to the GPU as SSBO. This memory is automatically mapped back to the cpu using
        /// VMA
        template <typename T>
        void push_buffer(const std::span<const T> buffer, std::string name)
        {
            auto gpu_buffer = std::make_unique<MemoryBuffer>();
            const size_t byte_size = buffer.size() * sizeof(T);

            VkBufferCreateInfo buffer_info = {};
            buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_info.size = byte_size;
            buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

            // Let VMA decide on the best memory usage as well as automatically mapping the data back
            VmaAllocationCreateInfo alloc_info{};
            alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
            alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

            // Create the buffer and copy our CPU data into the mapped memory
            vmaCreateBuffer(this->vk_instance.allocator, &buffer_info, &alloc_info, &gpu_buffer->buffer, &gpu_buffer->allocation, &gpu_buffer->allocation_info);
            std::memcpy(gpu_buffer->allocation_info.pMappedData, reinterpret_cast<const void*>(buffer.data()), byte_size);

            // Store the buffer by the given name
            this->buffers[name] = std::move(buffer);
        }

        /// Retrieve the modified GPU memory and copy it back into the provided buffer. It is up to the caller to ensure
        /// that they are retrieving the correct buffer.
        template <typename T>
        void retrieve_buffer(std::span<T> buffer, std::string name)
        {
            const size_t byte_size = buffer.size() * sizeof(T);
            if (!this->buffers.contains(name))
            {
                PSAPI_LOG_ERROR("Vulkan", "Internal error: Invalid buffer name '%s' passed, this buffer was not yet pushed to the GPU", name.c_str());
            }

            const auto& gpu_info = this->buffers[name];
            // Ensure that we are not trying to read more than the mapped memory. Note: it could be that
            // the buffer is larger than what we initially requested if VMA sees fit.
            if (gpu_info->allocation_info.size <= byte_size)
            {
                PSAPI_LOG_ERROR(
                    "Vulkan",
                    "Retrieval of GPU memory cannot exceed mapped memory size. Expected at most %zu bytes but instead requested %zu bytes",
                    static_cast<size_t>(gpu_info->allocation_info.size),
                    byte_size
                );
            }

            // Finally memcpy into the given buffer.
            std::memcpy(reinterpret_cast<void*>(buffer.data()), gpu_info->allocation_info.pMappedData, byte_size);
        }

        /// Retrieve the modified GPU memory and copy it back into the provided buffer. It is up to the caller to ensure
        /// that they are retrieving the correct buffer.
        template <typename T>
        std::vector<T> retrieve_buffer(std::string name)
        {

        }

    private:

        /// Get the VkQueue handle from the device held by `vk_instance` storing it on `queue` 
        void get_queues();

        /// Create the given amount of descriptors, currently only VK_DESCRIPTOR_TYPE_STORAGE_BUFFER is supported
        /// by this function. Handles initialization of `descriptor_set`, `descriptor_pool` and `descriptor_set_layout`.
        void create_descriptor(uint32_t num_descriptors);

        /// Create the compute pipeline initializing both `pipeline_layout` and `compute_pipeline`.
        void create_compute_pipeline(const std::vector<std::byte>& spv_code);

        /// Create the command pool initializing both `command_pool` and `command_buffer`.
        void create_command_pool();

        /// Utility function to create a VkShaderModule for the given instance and the given spir-v bytecode returning it.
        ///
        /// \param vk_instance The initialized vulkan instance
        /// \param spv_code The bytecode of the spir-v shader file.
        /// 
        /// \returns the VkShaderModule instance for the shader.
        VkShaderModule create_shader_module(const std::vector<std::byte>& spv_code) const noexcept;
    };

}

PSAPI_NAMESPACE_END