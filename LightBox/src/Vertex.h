#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>

#include "Vector3.h"

namespace LightBox {
	struct Vertex
	{
		Vector3 pos;
		glm::vec3 col;

		static VkVertexInputBindingDescription GetBindingDescriptions();
		static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions();
	};
}
