#include "Swapchain.h"

#include <algorithm>
#include <iostream>

#include "Application.h"
#include "VulkanUtils.h"

namespace LightBox {
Swapchain::Swapchain(Device& device)
		: m_Device(device)
{
	CreateSwapchain();
	CreateRenderPass();
	CreateFramebuffers();
	CreateSyncObjects();
};
Swapchain::~Swapchain()
{
	VkDevice device = m_Device.GetDevice();
	VkAllocationCallbacks* allocator = m_Device.GetAllocator();

	CleanupSwapchain();
	vkDestroyRenderPass(device, m_RenderPass, allocator);

	// Destroy sync objects
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, imageAvailableSemaphore[i], allocator);
		vkDestroySemaphore(device, renderFinishedSemaphore[i], allocator);
		vkDestroyFence(device, inFlightFence[i], allocator);
	}
}
void Swapchain::CreateSwapchain()
{
	VkDevice device = m_Device.GetDevice();
	VkAllocationCallbacks* allocator = m_Device.GetAllocator();

	// SWAPCHAIN
	VkSwapchainCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.surface = m_Device.GetSurface();
	create_info.minImageCount = 4;
	create_info.imageFormat = m_SwapchainImageFormat;
	create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	create_info.imageExtent = GetSwapExtent();
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;
	VkResult err = vkCreateSwapchainKHR(device, &create_info, allocator, &m_Swapchain);
	check_vk_result(err);
	//m_SwapchainImageFormat = create_info.imageFormat;
	m_SwapchainExtent = create_info.imageExtent;

	// SWAPCHAIN IMAGES
	vkGetSwapchainImagesKHR(device, m_Swapchain, &m_SwapchainImageCount, nullptr);
	m_SwapchainImages = (VkImage*)malloc(m_SwapchainImageCount * sizeof(VkImage));
	vkGetSwapchainImagesKHR(device, m_Swapchain, &m_SwapchainImageCount, m_SwapchainImages);

	//  SWAPCHAIN IMAGEVIEWS
	m_SwapchainImageViews = (VkImageView*)malloc(m_SwapchainImageCount * sizeof(VkImageView));
	for (uint32_t i = 0; i < m_SwapchainImageCount; i++) {
		VkImageViewCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.image = m_SwapchainImages[i];
		create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		create_info.format = m_SwapchainImageFormat;
		create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		create_info.subresourceRange.baseMipLevel = 0;
		create_info.subresourceRange.levelCount = 1;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount = 1;

		err = vkCreateImageView(device, &create_info, allocator, &m_SwapchainImageViews[i]);
		check_vk_result(err);
	}
}
void Swapchain::CleanupSwapchain()
{
	VkDevice device = m_Device.GetDevice();
	VkAllocationCallbacks* allocator = m_Device.GetAllocator();

	for (uint32_t i = 0; i < m_SwapchainImageCount; i++)
		vkDestroyFramebuffer(device, m_SwapchainFramebuffers[i], allocator);
	free(m_SwapchainFramebuffers);

	for (uint32_t i = 0; i < m_SwapchainImageCount; i++)
		vkDestroyImageView(device, m_SwapchainImageViews[i], allocator);
	free(m_SwapchainImageViews);

	vkDestroySwapchainKHR(device, m_Swapchain, allocator);
	free(m_SwapchainImages);
}
void Swapchain::CreateFramebuffers()
{
	VkDevice device = m_Device.GetDevice();
	VkAllocationCallbacks* allocator = m_Device.GetAllocator();

	m_SwapchainFramebuffers = (VkFramebuffer*)malloc(m_SwapchainImageCount * sizeof(VkFramebuffer));
	for (uint32_t i = 0; i < m_SwapchainImageCount; i++)
	{
		VkFramebufferCreateInfo framebuffer_info{};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = m_RenderPass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = &m_SwapchainImageViews[i];
		framebuffer_info.width = m_SwapchainExtent.width;
		framebuffer_info.height = m_SwapchainExtent.height;
		framebuffer_info.layers = 1;

		VkResult err = vkCreateFramebuffer(device, &framebuffer_info, allocator, &m_SwapchainFramebuffers[i]);
		check_vk_result(err);
	}
}
void Swapchain::RecreateSwapchain()
{
	VkDevice device = m_Device.GetDevice();
	VkAllocationCallbacks* allocator = m_Device.GetAllocator();

	// Handles window minimization
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_Device.GetWindow(), &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(m_Device.GetWindow(), &width, &height);
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(device);

	CleanupSwapchain();
	CreateSwapchain();
	CreateFramebuffers();
}
void Swapchain::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_SwapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPass_info{};
	renderPass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPass_info.attachmentCount = 1;
	renderPass_info.pAttachments = &colorAttachment;
	renderPass_info.subpassCount = 1;
	renderPass_info.pSubpasses = &subpass;
	renderPass_info.dependencyCount = 1;
	renderPass_info.pDependencies = &dependency;

	VkResult err = vkCreateRenderPass(m_Device.GetDevice(), &renderPass_info, m_Device.GetAllocator(), &m_RenderPass);
	check_vk_result(err);
}
VkExtent2D Swapchain::GetSwapExtent()
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Device.GetPhysicalDevice(), m_Device.GetSurface(), &capabilities);

	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		std::cout << capabilities.minImageExtent.width << " " << capabilities.minImageExtent.height << '\n';
		std::cout << capabilities.maxImageExtent.width << " " << capabilities.maxImageExtent.height << '\n';

		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(m_Device.GetWindow(), &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}
void Swapchain::CreateSyncObjects()
{
	VkDevice device = m_Device.GetDevice();
	VkAllocationCallbacks* allocator = m_Device.GetAllocator();

	imageAvailableSemaphore = (VkSemaphore*)malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkSemaphore));
	renderFinishedSemaphore = (VkSemaphore*)malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkSemaphore));
	inFlightFence = (VkFence*)malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkFence));

	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info{};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkResult err;
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (!imageAvailableSemaphore[i] && !renderFinishedSemaphore && !inFlightFence)
			return;
		err = vkCreateSemaphore(device, &semaphore_info, allocator, &imageAvailableSemaphore[i]);
		check_vk_result(err);
		err = vkCreateSemaphore(device, &semaphore_info, allocator, &renderFinishedSemaphore[i]);
		check_vk_result(err);
		err = vkCreateFence(device, &fence_info, allocator, &inFlightFence[i]);
		check_vk_result(err);
	}
}
}