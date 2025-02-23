#include "GpuRenderer.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include <stdexcept>
#include <chrono>
#include <iostream>
#include <array>

#include "Vertex.h"
#include "VulkanUtils.h"
#include "Math/Vector3.h"


namespace LightBox {
	static UniformBufferObject ubo{};

	GpuRenderer::GpuRenderer(Device& device, Camera& camera, Viewport& viewport)
		: m_Device(device), m_Viewport(viewport), m_Camera(camera)
	{
		//m_Camera.OnResize(m_Device.GetSwapchain().GetSwapExtent().width, m_Device.GetSwapchain().GetSwapExtent().height);

		std::cout << "[VULKAN] Swapchain width: " << m_Device.GetSwapchain().GetSwapExtent().width << '\n';
		std::cout << "[VULKAN] Swapchain height: " << m_Device.GetSwapchain().GetSwapExtent().height << '\n';

		CreateDescriptorPool();
		CreateDescriptorSetLayout();

		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			m_UniformBuffers[i].CreateUniformBuffer();
		CreateDescriptorSets();

		CreateRenderPass();
		m_GraphicsPipeline = new Pipeline(m_Device, this);
	}
	GpuRenderer::~GpuRenderer() {
		m_GraphicsPipeline->~Pipeline();
		free(m_GraphicsPipeline);

		vkDestroyRenderPass(m_Device.Get(), m_RenderPass, m_Device.GetAllocator());

		vkDestroyDescriptorPool(m_Device.Get(), m_DescriptorPool, m_Device.GetAllocator());
		vkDestroyDescriptorSetLayout(m_Device.Get(), m_DescriptorSetLayout, m_Device.GetAllocator());
	}
	void GpuRenderer::RecordRenderingCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex)
	{
		Swapchain& swapchain = m_Device.GetSwapchain();

		//VkCommandBufferBeginInfo begin_info{};
		//begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//begin_info.flags = 0;
		//begin_info.pInheritanceInfo = nullptr;

		//VkResult err = vkBeginCommandBuffer(commandBuffer, &begin_info);
		//check_vk_result(err);

		VkRenderPassBeginInfo renderPass_info{};
		renderPass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPass_info.renderPass = m_RenderPass;
		renderPass_info.framebuffer = m_Viewport.GetFramebuffers()[imageIndex];
		int32_t x_offset = swapchain.GetExtent().width - m_Camera.m_ViewportWidth;
		int32_t y_offset = swapchain.GetExtent().height - m_Camera.m_ViewportHeight;

		renderPass_info.renderArea.offset = { 0, 0 };
		//renderPass_info.renderArea.extent = swapchain.GetExtent();
		renderPass_info.renderArea.extent.width = m_Camera.m_ViewportWidth;
		renderPass_info.renderArea.extent.height = m_Camera.m_ViewportHeight;
		renderPass_info.renderArea.extent.width = 1582;
		renderPass_info.renderArea.extent.height = 998;

		VkClearValue clearColor{ {{0.2f, 0.2f, 0.2f, 1.f}} };
		renderPass_info.clearValueCount = 1;
		renderPass_info.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPass_info, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->GetPipeline());

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		//viewport.width = static_cast<float>(swapchain.GetExtent().width);
		//viewport.height = static_cast<float>(swapchain.GetExtent().height);
		viewport.width = 1582;
		viewport.height = 998;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		//scissor.extent = swapchain.GetExtent();
		scissor.extent.width = 1582;
		scissor.extent.height = 998;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		VkBuffer vertexBuffers[1] = { m_Scene->m_VertexBuffer.Get() };
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_Scene->m_IndexBuffer.Get(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->GetPipelineLayout(), 0, 1,
			&m_DescriptorSets[m_CurrentFrame], 0, nullptr);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Scene->m_IndicesCount), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		//VkResult err = vkEndCommandBuffer(commandBuffer);
		//check_vk_result(err);

		UpdateUniformBuffer(m_CurrentFrame);
	}
	void GpuRenderer::CreateDescriptorPool() {
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		VkResult res = vkCreateDescriptorPool(m_Device.Get(), &poolInfo, m_Device.GetAllocator(), &m_DescriptorPool);
		check_vk_result(res);
	}
	void GpuRenderer::CreateDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		VkResult res = vkCreateDescriptorSetLayout(m_Device.Get(), &layoutInfo, m_Device.GetAllocator(), &m_DescriptorSetLayout);
	}
	void GpuRenderer::CreateDescriptorSets() {
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		m_DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		VkResult res = vkAllocateDescriptorSets(m_Device.Get(), &allocInfo, m_DescriptorSets.data());
		check_vk_result(res);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_UniformBuffers[i].Get();
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr;
			descriptorWrite.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(m_Device.Get(), 1, &descriptorWrite, 0, nullptr);
		}
	}
	void GpuRenderer::CreateRenderPass()
	{
		VkAttachmentDescription color_attachment{};
		color_attachment.format = m_ImageFormat;
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference color_ref{};
		color_ref.attachment = 0;
		color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_ref;

		std::array<VkSubpassDependency, 2> dependencies;
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		create_info.attachmentCount = 1;
		create_info.pAttachments = &color_attachment;
		create_info.subpassCount = 1;
		create_info.pSubpasses = &subpass;
		create_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
		create_info.pDependencies = dependencies.data();

		VkResult res = vkCreateRenderPass(m_Device.Get(), &create_info, m_Device.GetAllocator(), &m_RenderPass);
		check_vk_result(res);
	}
	void GpuRenderer::UpdateUniformBuffer(uint32_t currentImage)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		memcpy(m_UniformBuffers[currentImage].GetMappedBuffer(), &ubo, sizeof(ubo));

		Mat4 scale = Mat4::GetScaleMatrix(Vector3(1, 1, 1));
		//Mat4 rotation = Mat4::GetYRotationMatrix(time * 90.f);
		Mat4 translation = Mat4::GetTranslationMatrix(Vector3(0.f, 0.f, 1.f));

		Mat4 model = Mat4::Multiply(scale, translation);
		//Mat4 model = Mat4::Multiply(scale, rotation);
		//model = Mat4::Multiply(model, translation);
		
		ubo.model = model;
		ubo.view = m_Camera.GetView();
		ubo.proj = m_Camera.GetProjection();
	}
	void GpuRenderer::CreateCommandBuffers()
	{
		m_CommandBuffers = (VkCommandBuffer*)malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkCommandBuffer));

		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = m_Device.GetCommandPool();
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

		VkResult err = vkAllocateCommandBuffers(m_Device.Get(), &alloc_info, m_CommandBuffers);
	}
}