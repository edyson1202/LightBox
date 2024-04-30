#include "Viewport.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include <iostream>


#include "VulkanUtils.h"

namespace LightBox {
	Viewport::Viewport(Device& device, VkRenderPass& render_pass)
		: m_Device(device), m_ViewportRenderPass(render_pass)
	{
		m_ImageCount = m_Device.GetSwapchain().GetImageCount();

		CreateViewportImages();
		CreateViewportImageViews();
		CreateViewportFrameBuffers();
		CreateSampler();
		CreateImageDescriptorSets();
	}
	Viewport::~Viewport()
	{
		for (uint32_t i = 0; i < m_ViewportImages.size(); i++) {
			vkDestroyImage(m_Device.GetDevice(), m_ViewportImages[i], m_Device.GetAllocator());
			vkFreeMemory(m_Device.GetDevice(), m_ViewportImagesMemory[i], m_Device.GetAllocator());

			vkDestroyImageView(m_Device.GetDevice(), m_ViewportImageViews[i], m_Device.GetAllocator());
			vkDestroyFramebuffer(m_Device.GetDevice(), m_ViewportFramebuffers[i], m_Device.GetAllocator());
		}
		vkDestroySampler(m_Device.GetDevice(), m_Sampler, m_Device.GetAllocator());
	}
	void Viewport::CreateViewportImages()
	{
		m_ViewportImages.resize(m_ImageCount);
		m_ViewportImagesMemory.resize(m_ImageCount);

		for (uint32_t i = 0; i < m_ViewportImages.size(); i++) {
			VkImageCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			create_info.imageType = VK_IMAGE_TYPE_2D;
			create_info.format = m_ViewportImageFormat;
			create_info.extent.width = m_Device.GetSwapchain().GetExtent().width;
			create_info.extent.height = m_Device.GetSwapchain().GetExtent().height;
			create_info.extent.depth = 1;
			create_info.arrayLayers = 1;
			create_info.mipLevels = 1;
			create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			create_info.samples = VK_SAMPLE_COUNT_1_BIT;
			create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
			create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VkResult res = vkCreateImage(m_Device.GetDevice(), &create_info, m_Device.GetAllocator(), &m_ViewportImages[i]);
			check_vk_result(res);

			VkMemoryRequirements mem_req;
			vkGetImageMemoryRequirements(m_Device.GetDevice(), m_ViewportImages[i], &mem_req);
			VkMemoryAllocateInfo alloc_info{};
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.allocationSize = mem_req.size;
			alloc_info.memoryTypeIndex = m_Device.FindMemoryType(mem_req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			vkAllocateMemory(m_Device.GetDevice(), &alloc_info, m_Device.GetAllocator(), &m_ViewportImagesMemory[i]);
			vkBindImageMemory(m_Device.GetDevice(), m_ViewportImages[i], m_ViewportImagesMemory[i], 0);

			VkCommandBuffer cmd_buffer = m_Device.BeginSingleTimeCommands();
			InsertImageMemoryBarrier(cmd_buffer, m_ViewportImages[i],
				VK_ACCESS_TRANSFER_READ_BIT,
				VK_ACCESS_MEMORY_READ_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
			m_Device.EndSingleTimeCommands(cmd_buffer);
		}
	}
	void Viewport::CreateViewportImageViews()
	{
		m_ViewportImageViews.resize(m_Device.GetSwapchain().GetImageCount());
		for (uint32_t i = 0; i < m_ViewportImageViews.size(); i++) {
			VkImageViewCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			create_info.image = m_ViewportImages[i];
			create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			create_info.format = m_ViewportImageFormat;
			create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			create_info.subresourceRange.baseMipLevel = 0;
			create_info.subresourceRange.levelCount = 1;
			create_info.subresourceRange.baseArrayLayer = 0;
			create_info.subresourceRange.layerCount = 1;

			VkResult res = vkCreateImageView(m_Device.GetDevice(), &create_info, m_Device.GetAllocator(), &m_ViewportImageViews[i]);
			check_vk_result(res);
		}
	}
	void Viewport::CreateViewportFrameBuffers()
	{
		m_ViewportFramebuffers.resize(m_Device.GetSwapchain().GetImageCount());
		for (uint32_t i = 0; i < m_ViewportFramebuffers.size(); i++) {
			VkFramebufferCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			create_info.renderPass = m_ViewportRenderPass;
			create_info.attachmentCount = 1;
			create_info.pAttachments = &m_ViewportImageViews[i];
			create_info.width = m_Device.GetSwapchain().GetExtent().width;
			create_info.height = m_Device.GetSwapchain().GetExtent().height;
			create_info.layers = 1;

			VkResult res = vkCreateFramebuffer(m_Device.GetDevice(), &create_info, m_Device.GetAllocator(), &m_ViewportFramebuffers[i]);
			check_vk_result(res);
		}
	}
	void Viewport::CreateSampler()
	{
		VkSamplerCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		create_info.magFilter = VK_FILTER_LINEAR;
		create_info.minFilter = VK_FILTER_LINEAR;
		create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		create_info.anisotropyEnable = VK_FALSE;
		create_info.maxAnisotropy = 1.0f;
		create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		create_info.unnormalizedCoordinates = VK_FALSE;
		create_info.compareEnable = VK_FALSE;
		create_info.compareOp = VK_COMPARE_OP_ALWAYS;
		create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		create_info.mipLodBias = 0.0f;
		create_info.minLod = -1000.0f;
		create_info.maxLod = 1000.0f;

		VkResult res = vkCreateSampler(m_Device.GetDevice(), &create_info, m_Device.GetAllocator(), &m_Sampler);
		check_vk_result(res);
	}
	void Viewport::CreateImageDescriptorSets()
	{
		m_Dset.resize(m_Device.GetSwapchain().GetImageCount());
		for (uint32_t i = 0; i < m_Dset.size(); i++) {
			m_Dset[i] = ImGui_ImplVulkan_AddTexture(m_Sampler, m_ViewportImageViews[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}
	void Viewport::InsertImageMemoryBarrier(VkCommandBuffer cmdbuffer,
		VkImage image,
		VkAccessFlags srcAccessMask,
		VkAccessFlags dstAccessMask,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask,
		VkImageSubresourceRange subresourceRange)
	{
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.srcAccessMask = srcAccessMask;
		imageMemoryBarrier.dstAccessMask = dstAccessMask;
		imageMemoryBarrier.oldLayout = oldImageLayout;
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		vkCmdPipelineBarrier(
			cmdbuffer,
			srcStageMask,
			dstStageMask,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
	}
}