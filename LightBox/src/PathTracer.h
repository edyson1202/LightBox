#pragma once

#include "Device.h"
#include "Scene.h"
#include "RTPipeline.h"
#include "Camera.h"
#include "Viewport.h"
#include "Renderer2.h"
#include "Image.h"

namespace LightBox
{
	class RTPipeline;
	struct RTUniformBufferObject {
		Mat4 view;
		Mat4 proj;
	};
	class PathTracer
	{
	public:
		PathTracer(Device& device, Scene& scene, Camera& camera, Viewport& viewport);

		void OnResize(uint32_t width, uint32_t height);

		Scene& GetScene() const { return m_Scene; }

		void RecordRenderingCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex);
	private:
		void CreateDescriptorPool();
		void CreateDescriptorSetLayout();
		void CreateDescriptorSets();
		void UpdateUniformBuffer(uint32_t currentImage);

		void CreateRenderPass();
	private:
		Device& m_Device;
		
		Scene& m_Scene;
		Camera& m_Camera;
		Viewport& m_Viewport;

		std::shared_ptr<Image> m_FinalImage;

		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> m_DescriptorSets;

		RTPipeline m_Pipeline;

		Buffer m_UniformBuffers[MAX_FRAMES_IN_FLIGHT] = { Buffer(m_Device), Buffer(m_Device) };

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		VkFormat m_ImageFormat = VK_FORMAT_B8G8R8A8_SRGB;
		uint32_t m_CurrentFrame = 0;
	};
}
