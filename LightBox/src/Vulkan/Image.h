#pragma once

#include "vulkan/vulkan.h"

#include <string>

#include "Device.h"

namespace LightBox {
	class Image
	{
	public:
		Image(Device& device, std::string path);
		Image(Device& device, uint32_t width, uint32_t height, VkFormat format, const void* data = nullptr);
		~Image();

		void Release();

		void SetData(const void* data);
		void Resize(uint32_t width, uint32_t height);
		VkDescriptorSet GetDescriptorSet() const { return m_DescriptorSet; }
		VkImageView GetImageView() { return m_ImageView; }

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
	private:
		void AllocateMemory(uint64_t size);
		void TransitionImageLayout(VkImage image, VkFormat, VkImageLayout old_layout, VkImageLayout new_layout);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	private:
		uint32_t m_Width, m_Height;
		VkFormat m_Format;
		VkDeviceSize m_ImageSize;

		VkImage m_FinalImage = nullptr;
		VkDeviceMemory m_ImageMemory = nullptr;
		VkImageView m_ImageView = nullptr;
		VkSampler m_Sampler = nullptr;

		VkDescriptorSet m_DescriptorSet = nullptr;

		VkBuffer m_StagingBuffer = nullptr;
		VkDeviceMemory m_StagingBufferMemory = nullptr;

		Device& m_Device;
	};
}