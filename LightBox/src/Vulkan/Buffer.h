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
		Buffer();
		~Buffer();

		void SetSize(uint32_t size) { m_BufferSize = size; }

		void CreateBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_properties);
		void Update(void* data, uint32_t size);
		void* Map();
		void UnMap();
		void CreateVertexBuffer(const std::vector<Vertex>& vertices);
		void CreateIndexBuffer(const std::vector<uint32_t>& indices);
		void CreateUniformBuffer();

		VkBuffer& Get() { return m_Buffer; }
		VkDeviceMemory& GetMemory() { return m_Memory; }
		void* GetMappedBuffer() { return m_MappedMemory; }

	private:

		void CopyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size);

	private:
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_Memory = VK_NULL_HANDLE;
		void* m_MappedMemory = nullptr;

		uint64_t m_BufferSize = 0;

		Device& m_Device;
		VkAllocationCallbacks* m_Allocator;
	};
}
