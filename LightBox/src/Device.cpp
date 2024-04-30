#include "Device.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "vulkan/vulkan_core.h"

#include <iostream>
#include <assert.h>
#include <vector>

#include "VulkanUtils.h"

//#define IMGUI_UNLIMITED_FRAME_RATE
//#define _DEBUG
#ifdef _DEBUG
#define IMGUI_VULKAN_DEBUG_REPORT

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	(void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
	fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
	return VK_FALSE;
}
#endif // IMGUI_VULKAN_DEBUG_REPORT
VkDevice g_Device = VK_NULL_HANDLE;

namespace LightBox {
	std::vector<const char*> device_extensions = {
	VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
	VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
	VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
	VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
	VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	// HELPER FUNCTIONS
	bool AreExtensionsIncluded(std::vector<const char*>& a, std::vector<VkExtensionProperties>& b)
	{
		for (uint32_t i = 0; i < a.size(); i++) {
			bool flag = false;
			for (uint32_t j = 0; j < b.size(); j++) {
				if (strcmp(a[i], b[j].extensionName)) {
					flag = true;
					break;
				}
			}
			if (!flag)
				return false;
		}
		return true;
	}

	Device::Device(GLFWwindow* window) 
		: m_Window(window) 
	{
		CreateInstance();
		SelectPhysicalDevice();
		CreateLogicalDevice();
		CreateSurface();
		CreateCommandPool();
		m_Swapchain = new Swapchain(*this);

		g_Device = m_Device;
	}
	Device::~Device() {

		vkDestroySurfaceKHR(m_Instance, m_Surface, m_Allocator);
		vkDestroyCommandPool(m_Device, m_CommandPool, m_Allocator);

#ifdef IMGUI_VULKAN_DEBUG_REPORT
		// Remove the debug report callback
		auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugReportCallbackEXT");
		vkDestroyDebugReportCallbackEXT(m_Instance, m_DebugReport, m_Allocator);
#endif // IMGUI_VULKAN_DEBUG_REPORT

		vkDestroyDevice(m_Device, m_Allocator);
		vkDestroyInstance(m_Instance, m_Allocator);

	}
	void Device::CreateInstance()
	{
		// VULKAN INSTANCE
		{
			VkResult result;
			VkApplicationInfo app_info = {};
			app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			app_info.apiVersion = VK_API_VERSION_1_3;

			// Getting extension names required by GLFW for VULKAN integration
			uint32_t extension_count = 0;
			const char** extensions = glfwGetRequiredInstanceExtensions(&extension_count);
			VkInstanceCreateInfo instance_info = {};
			instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			instance_info.pApplicationInfo = &app_info;
			instance_info.enabledExtensionCount = extension_count;
			instance_info.ppEnabledExtensionNames = extensions;


#ifdef IMGUI_VULKAN_DEBUG_REPORT
			// Enabling validation layers
			const char* enabled_layers[1] = { "VK_LAYER_KHRONOS_validation" };
			instance_info.enabledLayerCount = 1;
			instance_info.ppEnabledLayerNames = enabled_layers;

			// Enable debug report extension (we need additional storage, so we duplicate the user array to add our new extension to it)
			const char** extensions_ext = (const char**)malloc(sizeof(const char*) * (extension_count + 1));
			memcpy(extensions_ext, extensions, extension_count * sizeof(const char*));
			extensions_ext[extension_count] = "VK_EXT_debug_report";
			instance_info.enabledExtensionCount = extension_count + 1;
			instance_info.ppEnabledExtensionNames = extensions_ext;

			// Create Vulkan Instance
			result = vkCreateInstance(&instance_info, m_Allocator, &m_Instance);
			check_vk_result(result);
			free(extensions_ext);

			// Get the function pointer (required for any extensions)
			auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugReportCallbackEXT");
			assert(vkCreateDebugReportCallbackEXT != NULL);

			// Setup the debug report callback
			VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
			debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
			debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
			debug_report_ci.pfnCallback = debug_report;
			debug_report_ci.pUserData = NULL;
			result = vkCreateDebugReportCallbackEXT(m_Instance, &debug_report_ci, m_Allocator, &m_DebugReport);
			check_vk_result(result);
#else
			// Create Vulkan Instance without any debug features
			VkInstance vkInstance;
			result = vkCreateInstance(&instance_info, m_Allocator, &m_Instance);
			check_vk_result(result);
#endif
			PrintVulkanInstanceSupportedExtensions();
		}
	}
	void Device::SelectPhysicalDevice()
	{
		// Query physical devices
		uint32_t gpu_count;
		vkEnumeratePhysicalDevices(m_Instance, &gpu_count, nullptr);
		std::cout << "[vulkan] Physical devices count: " << gpu_count << '\n';
		std::vector<VkPhysicalDevice> devices(gpu_count);
		VkResult res = vkEnumeratePhysicalDevices(m_Instance, &gpu_count, devices.data());
		check_vk_result(res);
		std::cout << gpu_count << "\n";

		// Select a discrete GPU
		for (uint32_t i = 0; i < gpu_count; i++)
		{
			uint32_t extension_count;
			res = vkEnumerateDeviceExtensionProperties(devices[i], nullptr, &extension_count, nullptr);
			check_vk_result(res);
			std::vector<VkExtensionProperties> extension_properties(extension_count);
			res = vkEnumerateDeviceExtensionProperties(devices[i], nullptr, &extension_count, extension_properties.data());
			check_vk_result(res);
			if (AreExtensionsIncluded(device_extensions, extension_properties)) {
				m_PhysicalDevice = devices[i];
				break;
			}
		}
	}
	void Device::CreateLogicalDevice()
	{
		// SELECT GRAPHICS QUEUE FAMILY INDEX
		uint32_t count;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &count, nullptr);
		std::vector<VkQueueFamilyProperties>properties(count);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &count, properties.data());
		for (uint32_t i = 0; i < count; i++)
			if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				m_GraphicsQueueFamilyIndex = i;
				break;
			}
		assert(m_GraphicsQueueFamilyIndex != (uint32_t)-1);

		// QUEUE 
		VkDeviceQueueCreateInfo queue_info = {};
		queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_info.queueFamilyIndex = m_GraphicsQueueFamilyIndex;
		queue_info.queueCount = 1;
		float priority = 1.f;
		queue_info.pQueuePriorities = &priority;

		// Get GPU ray tracing pipeline properties
		VkPhysicalDeviceProperties2 gpu_properties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR rt_pipe_properties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR };
		gpu_properties.pNext = &rt_pipe_properties;
		vkGetPhysicalDeviceProperties2(m_PhysicalDevice, &gpu_properties);

		VkPhysicalDeviceFeatures2 features2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		VkPhysicalDeviceVulkan12Features features13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		VkPhysicalDeviceVulkan12Features features12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		VkPhysicalDeviceVulkan11Features features11{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
		VkPhysicalDeviceAccelerationStructureFeaturesKHR as_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR rt_pipe_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
		
		features2.pNext = &features13;
		features13.pNext = &features12;
		features12.pNext = &features11;
		features11.pNext = &as_features;
		as_features.pNext = &rt_pipe_features;
		// Query supported features.
		vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &features2);

		// LOGICAL DEVICE
		VkDeviceCreateInfo device_info = {};
		device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_info.enabledExtensionCount = (uint32_t)device_extensions.size();
		device_info.ppEnabledExtensionNames = device_extensions.data();
		device_info.pEnabledFeatures = nullptr;
		device_info.pNext = &features2;
		device_info.queueCreateInfoCount = 1;
		device_info.pQueueCreateInfos = &queue_info;

		VkResult res = vkCreateDevice(m_PhysicalDevice, &device_info, m_Allocator, &m_Device);
		check_vk_result(res);
		vkGetDeviceQueue(m_Device, m_GraphicsQueueFamilyIndex, 0, &m_Queue);

		// Loading vkCmdTraceRaysKHR function pointer
		PFN_vkCmdTraceRaysKHR pfn_vkCmdTraceRaysKHR = (PFN_vkCmdTraceRaysKHR)vkGetDeviceProcAddr(m_Device, "vkCmdTraceRaysKHR");
		pfn_vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(m_Device, "vkGetAccelerationStructureBuildSizesKHR"));
		pfn_vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(m_Device, "vkCreateAccelerationStructureKHR"));
	}
	void Device::CreateSurface()
	{
		VkResult err = glfwCreateWindowSurface(m_Instance, m_Window, m_Allocator, &m_Surface);
		check_vk_result(err);

		uint32_t count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &count, nullptr);
		VkPresentModeKHR* present_modes = (VkPresentModeKHR*)malloc(count * sizeof(VkPresentModeKHR));
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &count, present_modes);
	}
	void Device::PrintVulkanInstanceSupportedExtensions() {
		uint32_t count;
		vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
		VkExtensionProperties* properties = (VkExtensionProperties*)malloc(count * sizeof(VkExtensionProperties));
		vkEnumerateInstanceExtensionProperties(nullptr, &count, properties);

		for (uint32_t i = 0; i < count; i++) {
			std::cout << i + 1 << ". " << properties[i].extensionName << '\n';
		}
	}
	void Device::CreateCommandPool()
	{
		VkCommandPoolCreateInfo commandPool_info{};
		commandPool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPool_info.queueFamilyIndex = m_GraphicsQueueFamilyIndex;

		VkResult err = vkCreateCommandPool(m_Device, &commandPool_info, m_Allocator, &m_CommandPool);
		check_vk_result(err);
	}
	uint32_t Device::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}
		throw std::runtime_error("Failed to find suitable memory type");
	}
	void Device::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_properties,
		VkBuffer& buffer, VkDeviceMemory& buffer_memory)
	{
		VkDevice& device = m_Device;
		VkAllocationCallbacks* allocator = m_Allocator;

		VkBufferCreateInfo buffer_info{};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size;
		buffer_info.usage = usage;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult res = vkCreateBuffer(device, &buffer_info, allocator, &buffer);
		check_vk_result(res);

		VkMemoryRequirements mem_req;
		vkGetBufferMemoryRequirements(device, buffer, &mem_req);

		VkMemoryAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem_req.size;
		alloc_info.memoryTypeIndex = FindMemoryType(mem_req.memoryTypeBits, mem_properties);

		VkMemoryAllocateFlagsInfo flags_info{};
		if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {	
			flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
			flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
			alloc_info.pNext = &flags_info;
		}

		res = vkAllocateMemory(device, &alloc_info, allocator, &buffer_memory);
		check_vk_result(res);

		res = vkBindBufferMemory(device, buffer, buffer_memory, 0);
		check_vk_result(res);
	}
	VkCommandBuffer Device::BeginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = m_CommandPool;
		alloc_info.commandBufferCount = 1;

		VkCommandBuffer command_buffer;
		vkAllocateCommandBuffers(m_Device, &alloc_info, &command_buffer);

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(command_buffer, &begin_info);

		return command_buffer;
	}
	void Device::EndSingleTimeCommands(VkCommandBuffer command_buffer)
	{
		vkEndCommandBuffer(command_buffer);

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;

		vkQueueSubmit(m_Queue, 1, &submit_info, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_Queue);

		vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &command_buffer);
	}
	uint64_t Device::get_buffer_device_address(VkBuffer buffer)
	{
		VkBufferDeviceAddressInfoKHR buffer_device_address_info{};
		buffer_device_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		buffer_device_address_info.buffer = buffer;
		return vkGetBufferDeviceAddressKHR(m_Device, &buffer_device_address_info);
	}
	VKAPI_ATTR void VKAPI_CALL Device::vkCmdTraceRaysKHR(
		VkCommandBuffer commandBuffer,
		const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
		const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
		const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
		const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
		uint32_t width,
		uint32_t height,
		uint32_t depth)
	{
		assert(pfn_vkCmdTraceRaysKHR);
		pfn_vkCmdTraceRaysKHR(
			commandBuffer,
			pRaygenShaderBindingTable,
			pMissShaderBindingTable,
			pHitShaderBindingTable,
			pCallableShaderBindingTable,
			width,
			height,
			depth);
	}
	VKAPI_ATTR void VKAPI_CALL Device::vkGetAccelerationStructureBuildSizesKHR(
		VkDevice                                    device,
		VkAccelerationStructureBuildTypeKHR         buildType,
		const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
		const uint32_t* pMaxPrimitiveCounts,
		VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo)
	{
		assert(pfn_vkGetAccelerationStructureBuildSizesKHR);
		pfn_vkGetAccelerationStructureBuildSizesKHR(
			device,
			buildType,
			pBuildInfo,
			pMaxPrimitiveCounts,
			pSizeInfo);
	}
	VKAPI_ATTR VkResult VKAPI_CALL Device::vkCreateAccelerationStructureKHR(
		VkDevice                                    device,
		const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkAccelerationStructureKHR* pAccelerationStructure)
	{
		assert(pfn_vkCreateAccelerationStructureKHR);
		return pfn_vkCreateAccelerationStructureKHR(
			device,
			pCreateInfo,
			pAllocator,
			pAccelerationStructure);
	}
	VKAPI_ATTR void VKAPI_CALL Device::vkCmdBuildAccelerationStructuresKHR(
		VkCommandBuffer                             commandBuffer,
		uint32_t                                    infoCount,
		const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
		const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos)
	{
		PFN_vkCmdBuildAccelerationStructuresKHR pfn_vkCmdBuildAccelerationStructuresKHR = 
			reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(m_Device, "vkCmdBuildAccelerationStructuresKHR"));
		assert(pfn_vkCmdBuildAccelerationStructuresKHR);
		pfn_vkCmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);

	}

}
// Defining VULKAN external functions
VKAPI_ATTR VkDeviceAddress VKAPI_CALL vkGetBufferDeviceAddressKHR(
	VkDevice                                    device,
	const VkBufferDeviceAddressInfo* pInfo)
{
	PFN_vkGetBufferDeviceAddressKHR pfn_vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(g_Device, "vkGetBufferDeviceAddressKHR"));

	assert(pfn_vkGetBufferDeviceAddressKHR);
	return pfn_vkGetBufferDeviceAddressKHR(device, pInfo);
}
VKAPI_ATTR VkDeviceAddress VKAPI_CALL vkGetAccelerationStructureDeviceAddressKHR(
	VkDevice                                    device,
	const VkAccelerationStructureDeviceAddressInfoKHR* pInfo)
{
	PFN_vkGetAccelerationStructureDeviceAddressKHR pfn_vkGetAccelerationStructureDeviceAddressKHR =
		reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(g_Device, "vkGetAccelerationStructureDeviceAddressKHR"));
	assert(pfn_vkGetAccelerationStructureDeviceAddressKHR);
	return pfn_vkGetAccelerationStructureDeviceAddressKHR(device, pInfo);
}
VKAPI_ATTR VkResult VKAPI_CALL vkGetRayTracingShaderGroupHandlesKHR(
	VkDevice                                    device,
	VkPipeline                                  pipeline,
	uint32_t                                    firstGroup,
	uint32_t                                    groupCount,
	size_t                                      dataSize,
	void* pData)
{
	PFN_vkGetRayTracingShaderGroupHandlesKHR pfn_vkGetRayTracingShaderGroupHandlesKHR =
		reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(g_Device, "vkGetRayTracingShaderGroupHandlesKHR"));
	assert(pfn_vkGetRayTracingShaderGroupHandlesKHR);

	return pfn_vkGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
}
VKAPI_ATTR VkResult VKAPI_CALL vkCreateRayTracingPipelinesKHR(
	VkDevice                                    device,
	VkDeferredOperationKHR                      deferredOperation,
	VkPipelineCache                             pipelineCache,
	uint32_t                                    createInfoCount,
	const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
	const VkAllocationCallbacks* pAllocator,
	VkPipeline* pPipelines)
{
	PFN_vkCreateRayTracingPipelinesKHR pfn_vkCreateRayTracingPipelinesKHR =
		reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(g_Device, "vkCreateRayTracingPipelinesKHR"));
	assert(pfn_vkCreateRayTracingPipelinesKHR);

	return pfn_vkCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount,
		pCreateInfos, pAllocator, pPipelines);
}

