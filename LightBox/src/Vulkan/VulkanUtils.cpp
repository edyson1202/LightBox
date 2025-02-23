#include "VulkanUtils.h"

#include <fstream>

#include "Application.h"

namespace LightBox {
	void check_vk_result(VkResult err)
	{
		if (err == 0)
			return;
		fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
		throw std::runtime_error("error");
		if (err < 0)
			abort();
	}

	void VK_CHECK(VkResult err)
	{
		if (err == 0)
			return;
		fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
		throw std::runtime_error("error");
		if (err < 0)
			abort();
	}

	void glfw_error_callback(int error, const char* description)
	{
		fprintf(stderr, "GLFW error %d: %s\n", error, description);
	}
}
