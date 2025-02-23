#pragma once

#include <vector>

#include "vulkan/vulkan.h"

#include "Vulkan/Swapchain.h"

struct GLFWwindow;

namespace LightBox {
	class Buffer;

	static VkDevice g_Device;
	static VkAllocationCallbacks* g_Allocator;

	class Device final {
	public:
		Device(GLFWwindow* window);
		~Device();

		static VkDevice GetDevice() { return g_Device; }
		static VkAllocationCallbacks* GetAllocator() { return g_Allocator; }

		VkInstance GetInstance() const { return m_Instance; }
		VkDevice& Get() { return m_Device; }
		VkPhysicalDevice& GetPhysicalDevice() { return m_PhysicalDevice; }
		//VkAllocationCallbacks* GetAllocator() const { return m_Allocator; }
		uint32_t GetGraphicsQueueFamilyIndex() const { return m_GraphicsQueueFamilyIndex; }
		VkQueue& GetGraphicsQueue() { return m_Queue; }
		VkCommandPool GetCommandPool() const { return m_CommandPool; }
		GLFWwindow* GetWindow() const { return m_Window; }
		Swapchain& GetSwapchain() const { return *m_Swapchain; }
		VkSurfaceKHR GetSurface() const { return m_Surface; }

		static bool IsPhysicalDeviceSupportsExtensions(const VkPhysicalDevice device, const std::vector<const char*>& exts);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void Device::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_properties, Buffer& buffer);
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_properties,
			VkBuffer& buffer, VkDeviceMemory& buffer_memory);
		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer command_buffer);
		uint64_t Device::get_buffer_device_address(VkBuffer buffer);
	private:
		void CreateInstance();
		void SelectPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSurface();
		void CreateCommandPool();
		void LoadExternalFunctionPointers();

		void PrintVulkanInstanceSupportedExtensions();
	private:
		VkInstance m_Instance = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		uint32_t m_GraphicsQueueFamilyIndex = (uint32_t)-1;
		VkQueue m_Queue = VK_NULL_HANDLE;

		GLFWwindow* m_Window;
		VkSurfaceKHR m_Surface;
		Swapchain* m_Swapchain;
		VkCommandPool m_CommandPool = VK_NULL_HANDLE;

		VkAllocationCallbacks* m_Allocator = VK_NULL_HANDLE;
		VkDebugReportCallbackEXT m_DebugReport = VK_NULL_HANDLE;
	};
}

inline PFN_vkGetAccelerationStructureBuildSizesKHR pfn_vkGetAccelerationStructureBuildSizesKHR = VK_NULL_HANDLE;
inline PFN_vkCreateAccelerationStructureKHR pfn_vkCreateAccelerationStructureKHR = VK_NULL_HANDLE;
inline PFN_vkCmdBuildAccelerationStructuresKHR pfn_vkCmdBuildAccelerationStructuresKHR = VK_NULL_HANDLE;

inline PFN_vkGetAccelerationStructureDeviceAddressKHR pfn_vkGetAccelerationStructureDeviceAddressKHR = VK_NULL_HANDLE;
inline PFN_vkGetBufferDeviceAddressKHR pfn_vkGetBufferDeviceAddressKHR = VK_NULL_HANDLE;

inline PFN_vkGetRayTracingShaderGroupHandlesKHR pfn_vkGetRayTracingShaderGroupHandlesKHR = VK_NULL_HANDLE;
inline PFN_vkCreateRayTracingPipelinesKHR pfn_vkCreateRayTracingPipelinesKHR = VK_NULL_HANDLE;
inline PFN_vkCmdTraceRaysKHR pfn_vkCmdTraceRaysKHR = VK_NULL_HANDLE;




