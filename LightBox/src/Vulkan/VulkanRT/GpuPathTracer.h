#pragma once

#include "Vulkan/Device.h"
#include "Vulkan/VulkanRT/RTPipeline.h"
#include "Camera.h"
#include "Viewport.h"
#include "Math/Mat4.h"
#include "Vulkan/GpuRenderer.h"
#include "Vulkan/Image.h"
#include "Vulkan/VulkanRT/VulkanRTScene.h"

namespace LightBox
{
	class RTPipeline;
	struct RTUniformBufferObject {
		Mat4 view;
		Mat4 proj;
	};
	class GpuPathTracer
	{
	public:
		GpuPathTracer(Device& device);

		void Init(VulkanRTScene* scene, Camera* camera);

		void OnResize(uint32_t width, uint32_t height);

		std::shared_ptr<Image>& GetFinalImage() { return m_FinalImage; }
		VkDescriptorSetLayout& GetDescriptorSetLayout() { return m_DescriptorSetLayout; }

		void RayTrace(const VkCommandBuffer& cmdBuffer, uint32_t imageIndex);
	private:
		void CreateDescriptorPool();
		void CreateDescriptorSetLayout();
		void CreateDescriptorSets();
		void UpdateStorageImageDescriptor() const;
		void UpdateUniformBuffer(uint32_t currentImage);

	private:
		Device& m_Device;
		
		VulkanRTScene* m_Scene;
		Camera* m_Camera;

		std::shared_ptr<Image> m_FinalImage;

		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> m_DescriptorSets;

		RTPipeline m_Pipeline;

		Buffer m_UniformBuffers[MAX_FRAMES_IN_FLIGHT];

		uint32_t m_CurrentFrame = 0;
	};
}
