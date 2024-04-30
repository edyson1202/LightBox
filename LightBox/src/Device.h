#pragma once

#include "vulkan/vulkan.h"

#include "Swapchain.h"

struct GLFWwindow;

namespace LightBox {
	class Device final {
	public:
		Device(GLFWwindow* window);
		~Device();

		VkInstance GetInstance() { return m_Instance; }
		VkDevice& GetDevice() { return m_Device; }
		VkPhysicalDevice& GetPhysicalDevice() { return m_PhysicalDevice; }
		VkAllocationCallbacks* GetAllocator() { return m_Allocator; }
		uint32_t GetGraphicsQueueFamilyIndex() { return m_GraphicsQueueFamilyIndex; }
		VkQueue& GetGraphicsQueue() { return m_Queue; }
		VkCommandPool GetCommandPool() { return m_CommandPool; }
		GLFWwindow* GetWindow() { return m_Window; }
		Swapchain& GetSwapchain() { return *m_Swapchain; }
		VkSurfaceKHR GetSurface() { return m_Surface; }

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_properties,
			VkBuffer& buffer, VkDeviceMemory& buffer_memory);
		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer command_buffer);
		uint64_t Device::get_buffer_device_address(VkBuffer buffer);

		// Vulkan Ray Tracing extension functions
		PFN_vkCmdTraceRaysKHR pfn_vkCmdTraceRaysKHR = VK_NULL_HANDLE;
		VKAPI_ATTR void VKAPI_CALL vkCmdTraceRaysKHR(
			VkCommandBuffer commandBuffer,
			const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
			const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
			const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
			const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
			uint32_t width,
			uint32_t height,
			uint32_t depth);
		PFN_vkGetAccelerationStructureBuildSizesKHR pfn_vkGetAccelerationStructureBuildSizesKHR;
		VKAPI_ATTR void VKAPI_CALL vkGetAccelerationStructureBuildSizesKHR(
			VkDevice                                    device,
			VkAccelerationStructureBuildTypeKHR         buildType,
			const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
			const uint32_t* pMaxPrimitiveCounts,
			VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo);
		PFN_vkCreateAccelerationStructureKHR pfn_vkCreateAccelerationStructureKHR;
		VKAPI_ATTR VkResult VKAPI_CALL vkCreateAccelerationStructureKHR(
			VkDevice                                    device,
			const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkAccelerationStructureKHR* pAccelerationStructure);
		VKAPI_ATTR void VKAPI_CALL vkCmdBuildAccelerationStructuresKHR(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    infoCount,
			const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
			const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos);
	private:
		void CreateInstance();
		void SelectPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSurface();
		void CreateCommandPool();

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
