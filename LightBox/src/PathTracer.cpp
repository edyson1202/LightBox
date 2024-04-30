#include "PathTracer.h"

#include <chrono>
#include <array>

#include "VulkanUtils.h"


namespace LightBox
{
	static UniformBufferObject RTubo{};

	PathTracer::PathTracer(Device& device, Scene& scene, Camera& camera, Viewport& viewport)
		: m_Device(device), m_Scene(scene), m_Pipeline(m_Device, *this), m_Camera(camera), m_Viewport(viewport)
	{
		m_Pipeline.CreateRTPipeline();

		CreateRenderPass();
		CreateDescriptorPool();
		CreateDescriptorSetLayout();
		CreateDescriptorSets();
	}
	void PathTracer::OnResize(uint32_t width, uint32_t height)
	{
		if (m_FinalImage) {
			// no resize necessary
			if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
				return;

			m_FinalImage->Resize(width, height);
		}
		else {
			m_FinalImage = std::make_shared<Image>(m_Device, width, height, VK_FORMAT_R8G8B8A8_UNORM);
		}
	}
	void PathTracer::RecordRenderingCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex)
	{
		Swapchain& swapchain = m_Device.GetSwapchain();

		//VkCommandBufferBeginInfo begin_info{};
		//begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//begin_info.flags = 0;
		//begin_info.pInheritanceInfo = nullptr;

		//VkResult err = vkBeginCommandBuffer(commandBuffer, &begin_info);
		//check_vk_result(err);

		auto& pipeline_properties = m_Pipeline.GetPipelineProperties();
		const uint32_t handle_size_aligned = m_Pipeline.aligned_size(pipeline_properties.shaderGroupHandleSize, pipeline_properties.shaderGroupHandleAlignment);

		VkStridedDeviceAddressRegionKHR raygen_shader_sbt_entry{};
		raygen_shader_sbt_entry.deviceAddress = m_Device.get_buffer_device_address(m_Pipeline.GetRayGenSBT().GetBuffer());
		raygen_shader_sbt_entry.stride = handle_size_aligned;
		raygen_shader_sbt_entry.size = handle_size_aligned;

		VkStridedDeviceAddressRegionKHR miss_shader_sbt_entry{};
		miss_shader_sbt_entry.deviceAddress = m_Device.get_buffer_device_address(m_Pipeline.GetMissSBT().GetBuffer());
		miss_shader_sbt_entry.stride = handle_size_aligned;
		miss_shader_sbt_entry.size = handle_size_aligned;

		VkStridedDeviceAddressRegionKHR hit_shader_sbt_entry{};
		hit_shader_sbt_entry.deviceAddress = m_Device.get_buffer_device_address(m_Pipeline.GetHitSBT().GetBuffer());
		hit_shader_sbt_entry.stride = handle_size_aligned;
		hit_shader_sbt_entry.size = handle_size_aligned;

		VkStridedDeviceAddressRegionKHR callable_shader_sbt_entry{};

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_Pipeline.GetPipeline());

		VkRenderPassBeginInfo renderPass_info{};
		renderPass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPass_info.renderPass = m_RenderPass;
		renderPass_info.framebuffer = m_Viewport.GetFramebuffers()[imageIndex];
		renderPass_info.renderArea.offset = { 0, 0 };
		renderPass_info.renderArea.extent = swapchain.GetExtent();
		VkClearValue clearColor{ {{0.2f, 0.2f, 0.2f, 1.f}} };
		renderPass_info.clearValueCount = 1;
		renderPass_info.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPass_info, VK_SUBPASS_CONTENTS_INLINE);
		//vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->GetPipeline());

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapchain.GetExtent().width);
		viewport.height = static_cast<float>(swapchain.GetExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapchain.GetExtent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		//VkBuffer vertexBuffers[1] = { m_VertexBuffer.GetBuffer() };
		//VkDeviceSize offsets[1] = { 0 };
		//(commandBuffer, 0, 1, vertexBuffers, offsets);
		//vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT16);

		//vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->GetPipelineLayout(), 0, 1,
		//	&m_DescriptorSets[m_CurrentFrame], 0, nullptr);
		//vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(cube_indices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		//VkResult err = vkEndCommandBuffer(commandBuffer);
		//check_vk_result(err);

		UpdateUniformBuffer(m_CurrentFrame);
	}
	void PathTracer::CreateRenderPass()
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

		VkResult res = vkCreateRenderPass(m_Device.GetDevice(), &create_info, m_Device.GetAllocator(), &m_RenderPass);
		check_vk_result(res);
	}
	void PathTracer::CreateDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 3> pool_sizes;
		pool_sizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		pool_sizes[0].descriptorCount = 1;

		pool_sizes[1].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		pool_sizes[1].descriptorCount = 1;

		pool_sizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_sizes[2].descriptorCount = 1;

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.maxSets = 1;
		pool_info.poolSizeCount = 1;
		pool_info.pPoolSizes = pool_sizes.data();

		VkResult res = vkCreateDescriptorPool(m_Device.GetDevice(), &pool_info,
			m_Device.GetAllocator(), &m_DescriptorPool);
		check_vk_result(res);
	}
	void PathTracer::CreateDescriptorSetLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		bindings.resize(3);
		VkDescriptorSetLayoutBinding acceleration_structure_layout_binding{};
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

		VkDescriptorSetLayoutBinding result_image_layout_binding{};
		bindings[1].binding = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		bindings[1].descriptorCount = 1;
		bindings[1].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

		VkDescriptorSetLayoutBinding uniform_buffer_binding{};
		bindings[2].binding = 2;
		bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[2].descriptorCount = 1;
		bindings[2].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

		VkDescriptorSetLayoutCreateInfo layout_info{};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
		layout_info.pBindings = bindings.data();

		VkResult res = vkCreateDescriptorSetLayout(m_Device.GetDevice(), &layout_info,
			m_Device.GetAllocator(), &m_DescriptorSetLayout);
		check_vk_result(res);
	}
	void PathTracer::CreateDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(1, m_DescriptorSetLayout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = layouts.data();

		VkResult res = vkAllocateDescriptorSets(m_Device.GetDevice(), &allocInfo, &m_DescriptorSets[0]);
		check_vk_result(res);

		// Setup the descriptor for binding our top level acceleration structure to the ray tracing shaders
		VkWriteDescriptorSetAccelerationStructureKHR descriptor_acceleration_structure_info{};
		descriptor_acceleration_structure_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		descriptor_acceleration_structure_info.accelerationStructureCount = 1;
		descriptor_acceleration_structure_info.pAccelerationStructures = &m_Scene.GetTLAS();

		VkWriteDescriptorSet acceleration_structure_write{};
		acceleration_structure_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		acceleration_structure_write.dstSet = m_DescriptorSets[0];
		acceleration_structure_write.dstBinding = 0;
		acceleration_structure_write.descriptorCount = 1;
		acceleration_structure_write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		// The acceleration structure descriptor has to be chained via pNext
		acceleration_structure_write.pNext = &descriptor_acceleration_structure_info;

		VkDescriptorImageInfo image_descriptor{};
		image_descriptor.imageView = m_FinalImage->GetImageView();
		image_descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkDescriptorBufferInfo buffer_descriptor{};
		buffer_descriptor.buffer = m_UniformBuffers[0].GetBuffer();
		buffer_descriptor.offset = 0;
		buffer_descriptor.range = sizeof(RTUniformBufferObject);

		VkWriteDescriptorSet result_image_write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		result_image_write.dstSet = m_DescriptorSets[0];
		result_image_write.dstBinding = 1;
		result_image_write.descriptorCount = 1;
		result_image_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		result_image_write.pImageInfo = &image_descriptor;

		VkWriteDescriptorSet uniform_buffer_write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		uniform_buffer_write.dstSet = m_DescriptorSets[0];
		uniform_buffer_write.dstBinding = 2;
		uniform_buffer_write.descriptorCount = 1;
		uniform_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniform_buffer_write.pBufferInfo = &buffer_descriptor;

		std::vector<VkWriteDescriptorSet> write_descriptor_sets = {
			acceleration_structure_write,
			result_image_write,
			uniform_buffer_write };
		vkUpdateDescriptorSets(m_Device.GetDevice(), static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, VK_NULL_HANDLE);
	}
	void PathTracer::UpdateUniformBuffer(uint32_t currentImage)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		memcpy(m_UniformBuffers[currentImage].GetMappedBuffer(), &RTubo, sizeof(RTubo));

		Mat4 scale = Mat4::GetScaleMatrix(Vector3(1, 1, 1));
		Mat4 rotation = Mat4::GetYRotationMatrix(time * 90.f);
		Mat4 translation = Mat4::GetTranslationMatrix(Vector3(0.f, 0.f, -1.f));

		Mat4 model = Mat4::Multiply(scale, translation);
		//Mat4 model = Mat4::Multiply(scale, rotation);
		//model = Mat4::Multiply(model, translation);

		RTubo.view = m_Camera.GetView();
		RTubo.proj = m_Camera.GetProjection();
	}
}