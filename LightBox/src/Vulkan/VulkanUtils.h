#pragma once

#include <vulkan/vulkan.h>

namespace LightBox {
	void check_vk_result(VkResult err);
	void VK_CHECK(VkResult err);
	//uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void glfw_error_callback(int error, const char* description);
}

