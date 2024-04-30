#include "Buffer.h"

#include <fstream>
#include <array>
#include <iostream>

#include "VulkanUtils.h"
#include "Application.h"
#include "Vertex.h"


namespace LightBox {
	Buffer::Buffer(Device& device) 
		: m_Device(device), m_Allocator(device.GetAllocator()) {}
	//Buffer::Buffer(Device& device, uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_properties)
	//	: m_Device(device), m_Allocator(device.GetAllocator())
	//{
	//	m_Device.CreateBuffer(size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, mem_properties, m_Buffer, m_Memory);
	//}
	Buffer::~Buffer() {
		VkDevice device = m_Device.GetDevice();
		VkAllocationCallbacks* allocator = m_Device.GetAllocator();

		vkDestroyBuffer(device, m_Buffer, allocator);
		vkFreeMemory(device, m_Memory, allocator);
	}
	void Buffer::CopyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer command_buffer = m_Device.BeginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;

		vkCmdCopyBuffer(command_buffer, srcBuffer, dstBuffer, 1, &copyRegion);

		m_Device.EndSingleTimeCommands(command_buffer);
	}
	void Buffer::CreateBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_properties)
	{
		m_BufferSize = size;
		m_Device.CreateBuffer(size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, mem_properties,
			m_Buffer, m_Memory);
	}
	void Buffer::Update(void* data, uint32_t size)
	{
		VkDevice device = m_Device.GetDevice();
		VkAllocationCallbacks* allocator = m_Device.GetAllocator();

		VkBuffer staggingBuffer;
		VkDeviceMemory staggingMemory;
		VkDeviceSize bufferSize = size;

		m_Device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staggingBuffer, staggingMemory);

		void* mapped_data;
		vkMapMemory(device, staggingMemory, 0, bufferSize, 0, &mapped_data);
		memcpy(mapped_data, data, (size_t)bufferSize);
		vkUnmapMemory(device, staggingMemory);

		CopyBuffer(staggingBuffer, m_Buffer, bufferSize);

		vkDestroyBuffer(device, staggingBuffer, allocator);
		vkFreeMemory(device, staggingMemory, allocator);
	}
	void* Buffer::Map()
	{
		vkMapMemory(m_Device.GetDevice(), m_Memory, 0, m_BufferSize, 0, &m_MappedMemory);
		return m_MappedMemory;
	}
	void Buffer::UnMap()
	{
		vkUnmapMemory(m_Device.GetDevice(), m_Memory);
		free(m_MappedMemory);
	}
	void Buffer::CreateVertexBuffer(const std::vector<Vertex>& vertices) {
		VkDevice device = m_Device.GetDevice();
		VkAllocationCallbacks* allocator = m_Device.GetAllocator();

		VkBuffer staggingBuffer;
		VkDeviceMemory staggingMemory;
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		m_Device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staggingBuffer, staggingMemory);

		void* data;
		vkMapMemory(device, staggingMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(device, staggingMemory);

		m_Device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Buffer, m_Memory);

		CopyBuffer(staggingBuffer, m_Buffer, bufferSize);

		vkDestroyBuffer(device, staggingBuffer, allocator);
		vkFreeMemory(device, staggingMemory, allocator);
	}
	void Buffer::CreateIndexBuffer(const std::vector<uint16_t>& indices) {
		VkDevice device = m_Device.GetDevice();
		VkAllocationCallbacks* allocator = m_Device.GetAllocator();

		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
		VkBuffer staggingBuffer;
		VkDeviceMemory staggingBufferMemory;

		m_Device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staggingBuffer, staggingBufferMemory);

		void* data;
		vkMapMemory(device, staggingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(device, staggingBufferMemory);

		m_Device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Buffer, m_Memory);

		CopyBuffer(staggingBuffer, m_Buffer, bufferSize);

		vkDestroyBuffer(device, staggingBuffer, allocator);
		vkFreeMemory(device, staggingBufferMemory, allocator);
	}
	void Buffer::CreateUniformBuffer() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		m_Device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_Buffer, m_Memory);

		vkMapMemory(m_Device.GetDevice(), m_Memory, 0, bufferSize, 0, &m_MappedMemory);
	}
}