#pragma once

#include "Vulkan/Device.h"
#include "Vulkan/Buffer.h"

#include <vector>

namespace LightBox
{
	class RTPipeline final
	{
	public:
		RTPipeline(Device& device);
		~RTPipeline() = default;

		void CreateRTPipeline(VkDescriptorSetLayout& descriptor_set_layout);

		uint32_t aligned_size(uint32_t value, uint32_t alignment);
		VkPipeline Get() const { return m_RTPipeline; }
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR& GetPipelineProperties() { return m_RTPipelineProperties; }
		Buffer& GetRayGenSBT() { return m_RayGenSBT; }
		Buffer& GetMissSBT() { return m_MissSBT; }
		Buffer& GetHitSBT() { return m_HitSBT; }
		VkPipelineLayout& GetLayout() { return m_RTPipelineLayout; }
	private:
		void CreateShaderBindingTable();

		VkShaderModule CreateShaderModule(const char* path, Device& device);
	private:
		Device& m_Device;

		VkPipeline m_RTPipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_RTPipelineLayout = VK_NULL_HANDLE;
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR  m_RTPipelineProperties{};

		std::vector<VkRayTracingShaderGroupCreateInfoKHR> m_ShaderGroups{};

		Buffer m_RayGenSBT;
		Buffer m_MissSBT;
		Buffer m_HitSBT;
	};
}
