#include "RTPipeline.h"

#include "vulkan/vulkan.h"

#include "Vulkan/Pipeline.h"
#include "Vulkan/VulkanUtils.h"
#include "GpuPathTracer.h"

namespace LightBox
{
	uint32_t RTPipeline::aligned_size(uint32_t value, uint32_t alignment)
	{
		return (value + alignment - 1) & ~(alignment - 1);
	}
	RTPipeline::RTPipeline(Device& device, GpuPathTracer& path_tracer)
		: m_Device(device), m_PathTracer(path_tracer)
	{
		// Get the ray tracing pipeline properties, which we'll need later on
		m_RTPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
		VkPhysicalDeviceProperties2 device_properties{};
		device_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		device_properties.pNext = &m_RTPipelineProperties;
		vkGetPhysicalDeviceProperties2(m_Device.GetPhysicalDevice(), &device_properties);
	}
	void RTPipeline::CreateRTPipeline()
	{
		// Create RT Pipeline Layout
		VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.setLayoutCount = 1;
		pipeline_layout_create_info.pSetLayouts = &m_PathTracer.GetScene().GetDescriptorSetLayout();

		VkResult res = (vkCreatePipelineLayout(m_Device.GetDevice(), &pipeline_layout_create_info,
			nullptr, &m_RTPipelineLayout));
		check_vk_result(res);

		// Setting up ray tracing shader groups
		std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
		// Ray Generation Group
		{
			const char* rgen_shader_path = "src/shaders/raygen.rgen.spv";

			VkPipelineShaderStageCreateInfo stage_info{};
			stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stage_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
			stage_info.module = CreateShaderModule(rgen_shader_path, m_Device);
			stage_info.pName = "main";

			shader_stages.push_back(stage_info);

			VkRayTracingShaderGroupCreateInfoKHR raygen_group{};
			raygen_group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			raygen_group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			raygen_group.generalShader = static_cast<uint32_t>(shader_stages.size()) - 1;
			raygen_group.closestHitShader = VK_SHADER_UNUSED_KHR;
			raygen_group.anyHitShader = VK_SHADER_UNUSED_KHR;
			raygen_group.intersectionShader = VK_SHADER_UNUSED_KHR;

			m_ShaderGroups.push_back(raygen_group);
		}
		// Ray Miss Group
		{
			const char* miss_shader_path = "src/shaders/miss.rmiss.spv";

			VkPipelineShaderStageCreateInfo stage_info{};
			stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stage_info.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
			stage_info.module = CreateShaderModule(miss_shader_path, m_Device);
			stage_info.pName = "main";

			shader_stages.push_back(stage_info);

			VkRayTracingShaderGroupCreateInfoKHR miss_group{};
			miss_group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			miss_group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			miss_group.generalShader = static_cast<uint32_t>(shader_stages.size()) - 1;
			miss_group.closestHitShader = VK_SHADER_UNUSED_KHR;
			miss_group.anyHitShader = VK_SHADER_UNUSED_KHR;
			miss_group.intersectionShader = VK_SHADER_UNUSED_KHR;

			m_ShaderGroups.push_back(miss_group);
		}
		// Ray closest hit group
		{
			const char* closest_hit_shader_path = "src/shaders/closesthit.rchit.spv";

			VkPipelineShaderStageCreateInfo stage_info{};
			stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stage_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
			stage_info.module = CreateShaderModule(closest_hit_shader_path, m_Device);
			stage_info.pName = "main";

			shader_stages.push_back(stage_info);
			
			VkRayTracingShaderGroupCreateInfoKHR closes_hit_group{};
			closes_hit_group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			closes_hit_group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
			closes_hit_group.generalShader = VK_SHADER_UNUSED_KHR;
			closes_hit_group.closestHitShader = static_cast<uint32_t>(shader_stages.size()) - 1;
			closes_hit_group.anyHitShader = VK_SHADER_UNUSED_KHR;
			closes_hit_group.intersectionShader = VK_SHADER_UNUSED_KHR;

			m_ShaderGroups.push_back(closes_hit_group);
		}
		// Create the Ray Tracing Pipeline
		VkRayTracingPipelineCreateInfoKHR rtpipeline_info{};
		rtpipeline_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		rtpipeline_info.stageCount = static_cast<uint32_t>(shader_stages.size());
		rtpipeline_info.pStages = shader_stages.data();
		rtpipeline_info.groupCount = static_cast<uint32_t>(m_ShaderGroups.size());
		rtpipeline_info.pGroups = m_ShaderGroups.data();
		rtpipeline_info.maxPipelineRayRecursionDepth = 1;
		rtpipeline_info.layout = m_RTPipelineLayout;
		res = vkCreateRayTracingPipelinesKHR(m_Device.GetDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE,
			1, &rtpipeline_info, m_Device.GetAllocator(), &m_RTPipeline);
		check_vk_result(res);
	}
	void RTPipeline::CreateShaderBindingTables()
	{
		const uint32_t group_count = static_cast<uint32_t>(m_ShaderGroups.size());
		const uint32_t group_handle_size = m_RTPipelineProperties.shaderGroupHandleSize;
		const uint32_t group_handle_size_aligned = aligned_size(
			m_RTPipelineProperties.shaderGroupHandleSize, m_RTPipelineProperties.shaderGroupHandleAlignment);
		//const uint32_t handle_alignment = m_RTPipelineProperties.shaderGroupHandleAlignment;

		const uint32_t sbt_size = group_count * group_handle_size_aligned;

		// Copy the pipeline's shader handles into a host buffer
		std::vector<uint8_t> shader_handle_storage(sbt_size);
		VkResult res = vkGetRayTracingShaderGroupHandlesKHR(m_Device.GetDevice(), m_RTPipeline, 0, 
			group_count, sbt_size, shader_handle_storage.data());
		check_vk_result(res);

		const VkBufferUsageFlags sbt_buffer_usage_flags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR |
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		const VkMemoryPropertyFlags stb_memory_property = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		// Create binding table buffers for each shader type
		m_RayGenSBT.CreateBuffer(group_handle_size, sbt_buffer_usage_flags, stb_memory_property);
		m_MissSBT.CreateBuffer(group_handle_size, sbt_buffer_usage_flags, stb_memory_property);
		m_HitSBT.CreateBuffer(group_handle_size, sbt_buffer_usage_flags, stb_memory_property);

		// Copy the shader handles from the host buffer to the binding tables
		uint8_t* data = static_cast<uint8_t*>(m_RayGenSBT.Map());
		memcpy(data, shader_handle_storage.data(), group_handle_size);
		data = static_cast<uint8_t*>(m_RayGenSBT.Map());
		memcpy(data, shader_handle_storage.data() + group_handle_size_aligned, group_handle_size);
		data = static_cast<uint8_t*>(m_RayGenSBT.Map());
		memcpy(data, shader_handle_storage.data() + group_handle_size_aligned * 2, group_handle_size);

		m_RayGenSBT.UnMap();
		m_MissSBT.UnMap();
		m_HitSBT.UnMap();
	}
	VkShaderModule RTPipeline::CreateShaderModule(const char* path, Device& device)
	{
		uint32_t shader_code_size;
		uint32_t* shader_code = Pipeline::ReadShaderCode(path, shader_code_size);
		return Pipeline::CreateShaderModule(m_Device, shader_code, shader_code_size);
	}
}