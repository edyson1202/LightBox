#pragma once

#include <vulkan/vulkan.h>


namespace LightBox {
	class Application;
	class Device;
	class Swapchain
	{
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		Swapchain(Device& device);
		~Swapchain();

		Swapchain(const Swapchain&) = delete;
		Swapchain& operator=(const Swapchain&) = delete;

		void CreateSwapchain();
		void CleanupSwapchain();
		void RecreateSwapchain();
		void CreateFramebuffers();
		void CreateRenderPass();

		uint32_t GetMinImageCount() { return m_SwapchainMinImageCount; }
		uint32_t GetImageCount() { return m_SwapchainImageCount; }
		VkExtent2D GetExtent() { return m_SwapchainExtent; }

		VkRenderPass* GetRenderPass() { return &m_RenderPass; }

		VkSwapchainKHR* GetSwapchain() { return &m_Swapchain; };
		VkExtent2D GetSwapExtent();

		VkFramebuffer* m_SwapchainFramebuffers = VK_NULL_HANDLE;

		VkSemaphore* imageAvailableSemaphore;
		VkSemaphore* renderFinishedSemaphore;
		VkFence* inFlightFence;
	private:
		void CreateSyncObjects();	
	private:
		Device& m_Device;

		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		VkFormat m_SwapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;
		//VkFormat m_SwapchainImageFormat = VK_FORMAT_R8G8B8A8_UINT;
		VkExtent2D m_SwapchainExtent;
		uint32_t m_SwapchainImageCount;
		uint32_t m_SwapchainMinImageCount = 3;

		VkImage* m_SwapchainImages = VK_NULL_HANDLE;
		VkImageView* m_SwapchainImageViews = VK_NULL_HANDLE;	

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
	};
}
