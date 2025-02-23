#pragma once

#include <cstdint>

#include "Buffer.h"
#include "Swapchain.h"
#include "Pipeline.h"
#include "Device.h"
#include "Math/Mat4.h"
#include "Camera.h"
#include "Scene.h"
#include "Viewport.h"
#include "VulkanScene.h"

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

namespace LightBox {
	struct UniformBufferObject {
		Mat4 model;
		Mat4 view;
		Mat4 proj;
	};
	// TODO improve GpuRenderer memory layout
	class GpuRenderer {
	public:
		GpuRenderer(Device& device, Camera& camera, Viewport& viewport);
		~GpuRenderer();

		void SetScene(VulkanScene& scene) { m_Scene = &scene; }

		void RecordRenderingCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex);

		Pipeline* GetPipelineWrapper() { return m_GraphicsPipeline; }
		VkDescriptorSetLayout* GetDescriptorSetLayout() { return &m_DescriptorSetLayout; }
		VkRenderPass& GetRenderPass() { return m_RenderPass; }
	private:
		void UpdateUniformBuffer(uint32_t currentImage);

		void CreateDescriptorPool();
		void CreateDescriptorSetLayout();
		void CreateDescriptorSets();
		void CreateRenderPass();

		void CreateCommandBuffers();		
	private:
		VulkanScene* m_Scene;

		uint32_t m_CurrentFrame = 0;

		Device& m_Device;
		Pipeline* m_GraphicsPipeline;

		Viewport& m_Viewport;
		Camera& m_Camera;

		Buffer m_UniformBuffers[MAX_FRAMES_IN_FLIGHT];
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		VkFormat m_ImageFormat = VK_FORMAT_B8G8R8A8_SRGB;

		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> m_DescriptorSets;

		VkCommandBuffer* m_CommandBuffers = VK_NULL_HANDLE;
	};
}
