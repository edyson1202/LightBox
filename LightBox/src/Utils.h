#pragma once

#include <fstream>
#include <iostream>
#include <cmath>
#include <limits>
#include <memory>

#include <vulkan/vulkan.h>


namespace LightBox {
	class Utils
	{
	public:
		//static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		//static void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_properties,
		//	VkBuffer& buffer, VkDeviceMemory& buffer_memory);
		static void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& memory);	
	};

	// Constants

	const double infinity = std::numeric_limits<double>::infinity();
	const double pi = 3.1415926535897932385;

	// Utility Functions

	inline double degrees_to_radians(double degrees) {
		return degrees * pi / 180.0;
	}

}

