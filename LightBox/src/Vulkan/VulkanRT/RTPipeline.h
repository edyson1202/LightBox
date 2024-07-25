#pragma once

#include "Vulkan/Device.h"
#include "Vulkan/Buffer.h"

#include <vector>

namespace LightBox
{
	class GpuPathTracer;
	class RTPipeline final
	{
	public:
		RTPipeline(Device& device, GpuPathTracer& path_tracer);
		~RTPipeline() = default;

		void CreateRTPipeline();

		uint32_t aligned_size(uint32_t value, uint32_t alignment);
		VkPipeline GetPipeline() { return m_RTPipeline; }
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR& GetPipelineProperties() { return m_RTPipelineProperties; }
		Buffer& GetRayGenSBT() { return m_RayGenSBT; }
		Buffer& GetMissSBT() { return m_MissSBT; }
		Buffer& GetHitSBT() { return m_HitSBT; }
	private:
		void CreateShaderBindingTables();

		VkShaderModule CreateShaderModule(const char* path, Device& device);
	private:
		Device& m_Device;
		GpuPathTracer& m_PathTracer;

		VkPipeline m_RTPipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_RTPipelineLayout = VK_NULL_HANDLE;
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR  m_RTPipelineProperties{};

		std::vector<VkRayTracingShaderGroupCreateInfoKHR> m_ShaderGroups{};

		Buffer m_RayGenSBT = Buffer(m_Device);
		Buffer m_MissSBT = Buffer(m_Device);
		Buffer m_HitSBT = Buffer(m_Device);
	};
}
