#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <vector>

#include "Device.h"

namespace LightBox {
	struct Vertex;
	class Buffer final
	{
	public:
		Buffer(Device& device);
		//Buffer(Device& device, uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_properties);
		~Buffer();

		void CreateBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_properties);
		void Update(void* data, uint32_t size);
		void* Map();
		void UnMap();
		void CreateVertexBuffer(const std::vector<Vertex>& vertices);
		void CreateIndexBuffer(const std::vector<uint16_t>& indices);
		void CreateUniformBuffer();

		VkBuffer& GetBuffer() { return m_Buffer; }
		VkDeviceMemory& GetMemory() { return m_Memory; }
		void* GetMappedBuffer() { return m_MappedMemory; }
		//void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_properties,
		//	VkBuffer& buffer, VkDeviceMemory& buffer_memory);

	private:
		

		void CopyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size);
	private:
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_Memory = VK_NULL_HANDLE;
		void* m_MappedMemory = nullptr;

		uint64_t m_BufferSize = 0;

		//VkBufferUsageFlags m_UsageFlags;
		//VkMemoryPropertyFlags m_MemoryFlags;

		Device& m_Device;
		VkAllocationCallbacks* m_Allocator;
	};
}
