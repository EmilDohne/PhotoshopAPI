#include "VkPrimitives.h"

#include "Macros.h"
#include "Logger.h"
#include "Core/Struct/File.h"
#include "Core/FileIO/Read.h"

#include <vk_mem_alloc.h>

#include <VkBootstrap.h>
#include <vulkan/vulkan.h>

PSAPI_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void Vulkan::InitInstance::init()
{
	vkb::InstanceBuilder instance_builder;
	auto ret = instance_builder.use_default_debug_messenger()
		.request_validation_layers()
		.set_headless()	// Since we only worry about the compute pipeline we don't need a swapchain
		.build();
	if (!ret)
	{
		PSAPI_LOG_ERROR("Vulkan", "Failed to build vulkan instance with the following error: %s", ret.error().message().c_str());
	}

	// Get the instance and dispatch table from the builder
	this->instance = ret.value();
	this->instance_dispatch = this->instance.make_table();

	// Grab the most appropriate physical device, vk-bootstrap will default to discrete GPUs.
	// we may at some point want to allow multi-gpu users to select the instance.
	vkb::PhysicalDeviceSelector selector(this->instance);
	auto phys_device_ret = selector.select();
	if (!phys_device_ret)
	{
		PSAPI_LOG_ERROR("Vulkan", "%s", phys_device_ret.error().message().c_str());
	}
	vkb::PhysicalDevice physical_device = phys_device_ret.value();

	vkb::DeviceBuilder device_builder{ physical_device };
	auto device_ret = device_builder.build();
	if (!device_ret)
	{
		PSAPI_LOG_ERROR("Vulkan", "%s", phys_device_ret.error().message().c_str());
	}
	
	this->device = device_ret.value();
	this->dispatch = this->device.make_table();

	// Setup VMA (Vulkan MemoryAllocator) for the instance
	this->init_vma();

	this->m_Initialized = true;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void Vulkan::InitInstance::init_vma()
{
	VmaVulkanFunctions vulkan_functions = {};
	vulkan_functions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
	vulkan_functions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

	VmaAllocatorCreateInfo alloc_create_info = {};
	alloc_create_info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
	alloc_create_info.vulkanApiVersion = VK_API_VERSION_1_2;
	alloc_create_info.physicalDevice = this->device.physical_device;
	alloc_create_info.device = this->device.device;
	alloc_create_info.instance = this->instance.instance;
	alloc_create_info.pVulkanFunctions = &vulkan_functions;

	vmaCreateAllocator(&alloc_create_info, &this->allocator);
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
bool Vulkan::InitInstance::is_initialized() const noexcept
{
	return m_Initialized;
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
Vulkan::InitInstance::~InitInstance()
{
	vkb::destroy_device(this->device);
	vkb::destroy_instance(this->instance);

	vmaDestroyAllocator(allocator);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
Vulkan::BaseData::BaseData(std::unique_ptr<Vulkan::InitInstance> instance, std::filesystem::path spv_path, uint32_t num_descriptors)
{
	if (!instance->is_initialized())
	{
		PSAPI_LOG_ERROR("Vulkan", "Passed uninitialized VkInit instance to VkBaseData");
	}
	this->vk_instance = std::move(instance);

	auto spv_file = File(spv_path);
	auto spv_bytes = ReadBinaryArray<std::byte>(spv_file, spv_file.size());

	this->create_descriptor(num_descriptors);
	this->create_compute_pipeline(spv_bytes);
	this->create_command_pool();
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void Vulkan::BaseData::create_and_submit()
{
	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	const auto workgroups = this->compute_workgroup_sizes();

	this->vk_instance->dispatch.beginCommandBuffer(this->command_buffer, &begin_info);
	this->vk_instance->dispatch.cmdBindPipeline(this->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, this->compute_pipeline);
	this->vk_instance->dispatch.cmdBindDescriptorSets(this->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, this->pipeline_layout, 0, 1, &this->descriptor_set, 0, nullptr);
	this->vk_instance->dispatch.cmdDispatch(this->command_buffer, workgroups[0], workgroups[1], workgroups[2]);
	this->vk_instance->dispatch.endCommandBuffer(this->command_buffer);

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &this->command_buffer;
	this->vk_instance->dispatch.queueSubmit(this->queue, 1, &submit_info, VK_NULL_HANDLE);

	// Block until the computation is done since we don't need this to run async or anything like that.
	this->vk_instance->dispatch.deviceWaitIdle();
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void Vulkan::BaseData::get_queues()
{
	auto graphics_queue = this->vk_instance->device.get_queue(vkb::QueueType::graphics);
	if (!graphics_queue)
	{
		PSAPI_LOG_ERROR("Vulkan", "Failed to initialize graphics queue: %s", graphics_queue.error().message().c_str());
	}
	this->queue = graphics_queue.value();
}



// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void Vulkan::BaseData::create_descriptor(uint32_t num_descriptors)
{
	VkDescriptorPoolSize pool_sizes = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, num_descriptors };

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = 0;
	pool_info.maxSets = 1;
	pool_info.poolSizeCount = 1;
	pool_info.pPoolSizes = &pool_sizes;
	this->vk_instance->dispatch.createDescriptorPool(&pool_info, nullptr, &this->descriptor_pool);

	VkDescriptorSetLayoutBinding binding = { 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, num_descriptors, VK_SHADER_STAGE_ALL, nullptr };
	VkDescriptorSetLayoutCreateInfo dsl_info = {};
	dsl_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	dsl_info.flags = 0;
	dsl_info.bindingCount = 1;
	dsl_info.pBindings = &binding;
	this->vk_instance->dispatch.createDescriptorSetLayout(&dsl_info, nullptr, &this->descriptor_set_layout);

	VkDescriptorSetAllocateInfo ds_allocate_info = {};
	ds_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	ds_allocate_info.descriptorPool = this->descriptor_pool;
	ds_allocate_info.descriptorSetCount = 1;
	ds_allocate_info.pSetLayouts = &this->descriptor_set_layout;
	this->vk_instance->dispatch.allocateDescriptorSets(&ds_allocate_info, &this->descriptor_set);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void Vulkan::BaseData::create_compute_pipeline(const std::vector<std::byte>& spv_code)
{
	VkShaderModule shader_module = this->create_shader_module(spv_code);

	VkPipelineShaderStageCreateInfo shader_stage_info = {};
	shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shader_stage_info.module = shader_module;
	shader_stage_info.pName = "main";

	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &this->descriptor_set_layout;
	pipeline_layout_info.pushConstantRangeCount = 0;
	this->vk_instance->dispatch.createPipelineLayout(&pipeline_layout_info, nullptr, &this->pipeline_layout);

	VkComputePipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipeline_info.stage = shader_stage_info;
	pipeline_info.layout = this->pipeline_layout;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	this->vk_instance->dispatch.createComputePipelines(VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &this->compute_pipeline);

	this->vk_instance->dispatch.destroyShaderModule(shader_module, nullptr);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void Vulkan::BaseData::create_command_pool()
{
	VkCommandPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = this->vk_instance->device.get_queue_index(vkb::QueueType::graphics).value();
	this->vk_instance->dispatch.createCommandPool(&pool_info, nullptr, &this->command_pool);

	VkCommandBufferAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocate_info.commandPool = this->command_pool;
	allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocate_info.commandBufferCount = 1;
	this->vk_instance->dispatch.allocateCommandBuffers(&allocate_info, &this->command_buffer);
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
VkShaderModule Vulkan::BaseData::create_shader_module(const std::vector<std::byte>& spv_code) const noexcept
{
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = spv_code.size();
	create_info.pCode = reinterpret_cast<const uint32_t*>(spv_code.data());

	VkShaderModule shader_module;
	this->vk_instance->dispatch.createShaderModule(&create_info, nullptr, &shader_module);

	return shader_module;
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
Vulkan::BaseData::~BaseData()
{
	this->vk_instance->dispatch.destroyCommandPool(this->command_pool, nullptr);
	this->vk_instance->dispatch.destroyPipeline(this->compute_pipeline, nullptr);
	this->vk_instance->dispatch.destroyPipelineLayout(this->pipeline_layout, nullptr);

	for (const auto& [buffer_name, memory] : this->buffers)
	{
		vmaDestroyBuffer(this->vk_instance->allocator, memory->buffer, memory->allocation);
	}

	this->vk_instance->dispatch.destroyDescriptorPool(this->descriptor_pool, nullptr);
	this->vk_instance->dispatch.destroyDescriptorSetLayout(this->descriptor_set_layout, nullptr);

	// Since we hold this->vk_instance the destructor would be called here destroying the instance and 
	// device.
}



PSAPI_NAMESPACE_END