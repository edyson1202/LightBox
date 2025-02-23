#include "Image.h"

#include "backends/imgui_impl_vulkan.h"
#include <stb_image.h>

#include "Application.h"
#include "Utils.h"
#include "VulkanUtils.h"
#include "Buffer.h"

namespace LightBox {
	Image::Image(Device& device, std::string& path)
		: m_Device(device)
	{
		std::string file_path = path;
		uint8_t* data = nullptr;
		int width, height, channels;

		data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

		if (!data) {
			throw std::runtime_error("failed to load texture image!");
		}
		m_Width = width;
		m_Height = height;
		m_Format = VK_FORMAT_R8G8B8A8_UNORM;
		m_ImageSize = m_Width * m_Height * 4;

		AllocateMemory(m_ImageSize);
		SetData(data);
		stbi_image_free(data);
	}
	Image::Image(Device& device, uint32_t width, uint32_t height, VkFormat format, const void* data)
		: m_Device(device), m_Width(width), m_Height(height), m_Format(format)
	{
		m_ImageSize = m_Width * m_Height * 4;
		AllocateMemory(m_ImageSize);
		if (data) {
			SetData(data);
		}

		std::cout << "[VULKAN] Image constructor called!\n";
	}
	Image::~Image()
	{
		Release();

		std::cout << "[VULKAN] Image Destructor called!\n";
	}
	void Image::Release() {
		//Application::SubmitResourceFree([sampler = m_Sampler, image_view = m_ImageView, image = m_FinalImage,
		//	image_memory = m_ImageMemory, buffer = m_StagingBuffer, buffer_memory = m_StagingBufferMemory, 
		//	device = m_Device.GetDevice(), allocator = m_Device.GetAllocator()]()
		//	{
		//		vkDestroySampler(device, sampler, allocator);
		//		vkDestroyImageView(device, image_view, allocator);
		//		vkDestroyImage(device, image, allocator);
		//		vkFreeMemory(device, image_memory, allocator);
		//		vkDestroyBuffer(device, buffer, allocator);
		//		vkFreeMemory(device, buffer_memory, allocator);

		//	});

		m_Sampler = nullptr;
		m_ImageView = nullptr;
		m_FinalImage = nullptr;
		m_ImageMemory = nullptr;
		m_StagingBuffer = nullptr;
		m_StagingBufferMemory = nullptr;
	}

	void Image::AllocateMemory(uint64_t size)
	{
		VkDevice device = m_Device.Get();
		VkAllocationCallbacks* allocator = m_Device.GetAllocator();

		VkResult res;

		// Create the image
		{
			VkImageCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			info.imageType = VK_IMAGE_TYPE_2D;
			info.format = m_Format;
			info.extent.width = m_Width;
			info.extent.height = m_Height;
			info.extent.depth = 1;
			info.mipLevels = 1;
			info.arrayLayers = 1;
			info.samples = VK_SAMPLE_COUNT_1_BIT;
			info.tiling = VK_IMAGE_TILING_OPTIMAL;
			// TODO VK_IMAGE_USAGE_STORAGE_BIT was added to accomodate Ray Tracing destination image, however image usage should be parametrized at image creation
			info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
			info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			res = vkCreateImage(device, &info, allocator, &m_FinalImage);
			check_vk_result(res);

			VkMemoryRequirements mem_req;
			vkGetImageMemoryRequirements(device, m_FinalImage, &mem_req);

			VkMemoryAllocateInfo alloc_info{};
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.allocationSize = mem_req.size;
			alloc_info.memoryTypeIndex = m_Device.FindMemoryType(mem_req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			res = vkAllocateMemory(device, &alloc_info, allocator, &m_ImageMemory);
			check_vk_result(res);
			res = vkBindImageMemory(device, m_FinalImage, m_ImageMemory, 0);
			check_vk_result(res);
		}
		// Create the image view
		{
			VkImageViewCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.image = m_FinalImage;
			info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			info.format = m_Format;
			info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			info.subresourceRange.levelCount = 1;
			info.subresourceRange.layerCount = 1;

			res = vkCreateImageView(device, &info, allocator, &m_ImageView);
			check_vk_result(res);

			std::cout << "[VULKAN] Image view created:" << res << '\n';
		}
		// Create the sampler
		{
			VkSamplerCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.magFilter = VK_FILTER_LINEAR;
			info.minFilter = VK_FILTER_LINEAR;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.minLod = -1000;
			info.maxLod = 1000;
			info.maxAnisotropy = 1.0f;

			res = vkCreateSampler(device, &info, allocator, &m_Sampler);
			check_vk_result	(res);
		}

		m_DescriptorSet = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(m_Sampler, m_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void Image::TransitionImageLayout(VkImageLayout old_layout, VkImageLayout new_layout)
	{
		TransitionImageLayout(m_FinalImage, m_Format, old_layout, new_layout);
	}

	void Image::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
	{
		VkCommandBuffer command_buffer = m_Device.BeginSingleTimeCommands();

		VkPipelineStageFlags src_stage;
		VkPipelineStageFlags dst_stage;

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = old_layout;
		barrier.newLayout = new_layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
			throw std::invalid_argument("unsupported layout transition!");

		vkCmdPipelineBarrier(command_buffer, src_stage, dst_stage, 
			0, 
			0, nullptr, 
			0, nullptr, 
			1, &barrier);

		m_Device.EndSingleTimeCommands(command_buffer);
	}
	void Image::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer command_buffer = m_Device.BeginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		m_Device.EndSingleTimeCommands(command_buffer);
	}
	void Image::SetData(const void* data)
	{
		VkDevice device = m_Device.Get();

		if (!m_StagingBuffer) {
			m_Device.CreateBuffer(m_ImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_StagingBuffer, m_StagingBufferMemory);
		}
		void* map;
		vkMapMemory(device, m_StagingBufferMemory, 0, m_ImageSize, 0, &map);
		memcpy(map, data, m_ImageSize);
		vkUnmapMemory(device, m_StagingBufferMemory);

		TransitionImageLayout(m_FinalImage, m_Format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		CopyBufferToImage(m_StagingBuffer, m_FinalImage, m_Width, m_Height);
		TransitionImageLayout(m_FinalImage, m_Format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void Image::Resize(uint32_t width, uint32_t height)
	{
		if (m_FinalImage && m_Width == width && m_Height == height)
			return;

		// TODO max size?

		m_Width = width;
		m_Height = height;
		m_ImageSize = m_Width * m_Height * 4;

		Release();
		AllocateMemory(m_ImageSize);
	}

}

