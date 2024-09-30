#include "Utils.h"
#include "Application.h"
#include "Vulkan/VulkanUtils.h"

namespace LightBox {
	void TerminateIf(bool condition, const char* message)
	{
		if (condition)
		{
			std::cerr << "[ERROR] " << message << "\n";
			std::abort();
		}
	}
}
