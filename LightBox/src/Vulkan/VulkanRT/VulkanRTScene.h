#pragma once

#include "Vulkan/Buffer.h"
#include "Vulkan/Device.h"

namespace LightBox
{
	class VulkanRTScene
	{
	public:
		VulkanRTScene(Device& device);
		~VulkanRTScene();

		VkDescriptorSetLayout& GetDescriptorSetLayout() { return m_DescriptorSetLayout; }
		VkAccelerationStructureKHR& GetTLAS() { return m_TLAS; }
	private:
		void UploadSceneToGPU();
		void CreateAccelerationStructure();
		void CreateDescriptorPool();
		void CreateDescriptorSetLayout();
		void CreateDescriptorSets();
		//uint64_t get_buffer_device_address(VkBuffer buffer);
	private:
		Device& m_Device;

		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorSet m_DescriptorSets = VK_NULL_HANDLE;

		Buffer m_VertexBuffer;
		Buffer m_IndexBuffer;

		VkAccelerationStructureKHR m_Blas = VK_NULL_HANDLE;
		Buffer m_BLASBuffer;
		Buffer m_BLASScratchBuffer = Buffer(m_Device);
		Buffer m_InstancesBuffer = Buffer(m_Device);
		VkAccelerationStructureKHR m_TLAS = VK_NULL_HANDLE;
		Buffer m_TLASBuffer = Buffer(m_Device);
		Buffer m_TLASScratchBuffer = Buffer(m_Device);
	};
}
