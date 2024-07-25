#pragma once

#include "vulkan/vulkan.h"

#include <vector>

#include "Vulkan/Device.h"

namespace LightBox {
	class Viewport {
	public:
		Viewport(Device& device, VkRenderPass& render_pass);
		~Viewport();

		std::vector<VkFramebuffer>& GetFramebuffers() { return m_ViewportFramebuffers; }
		std::vector<VkDescriptorSet>& GetDescriptorSets() { return m_Dset; }
	private:
		void CreateViewportImages();
		void CreateViewportImageViews();
		void CreateViewportFrameBuffers();
		void CreateSampler();
		void CreateImageDescriptorSets();
		void InsertImageMemoryBarrier(VkCommandBuffer cmdbuffer,
			VkImage image,
			VkAccessFlags srcAccessMask,
			VkAccessFlags dstAccessMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask,
			VkImageSubresourceRange subresourceRange);
	private:
		Device& m_Device;
		VkRenderPass& m_ViewportRenderPass;

		uint32_t m_ImageCount;

		std::vector<VkImage> m_ViewportImages;
		std::vector<VkDeviceMemory> m_ViewportImagesMemory;
		std::vector<VkDescriptorSet> m_Dset;
		std::vector<VkImageView> m_ViewportImageViews;
		std::vector<VkFramebuffer> m_ViewportFramebuffers;
		VkSampler m_Sampler = VK_NULL_HANDLE;

		VkFormat m_ViewportImageFormat = VK_FORMAT_B8G8R8A8_SRGB;
	};
}
