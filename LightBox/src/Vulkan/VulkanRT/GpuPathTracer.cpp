#include "GpuPathTracer.h"

#include <chrono>
#include <array>

#include "Vulkan/VulkanUtils.h"

namespace LightBox
{
	static RTUniformBufferObject RTubo{};

	GpuPathTracer::GpuPathTracer(Device& device)
		: m_Device(device),
		m_Pipeline(m_Device)
	{
		m_FinalImage = std::make_shared<Image>(m_Device, 1400, 1000, VK_FORMAT_R8G8B8A8_UNORM);

		m_Device.CreateBuffer(sizeof(RTUniformBufferObject),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_UniformBuffers[0]);
		m_UniformBuffers[0].Map();

		CreateDescriptorPool();
		CreateDescriptorSetLayout();

		m_Pipeline.CreateRTPipeline(m_DescriptorSetLayout);
	}

	void GpuPathTracer::Init(VulkanRTScene* scene, Camera* camera)
	{
		m_Scene = scene;
		m_Camera = camera;

		CreateDescriptorSets();
	}

	void GpuPathTracer::OnResize(uint32_t width, uint32_t height)
	{
		if (m_FinalImage) {
			// no resize necessary
			if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
				return;

			m_FinalImage->Resize(width, height);
			UpdateStorageImageDescriptor();
		}
		else {
			m_FinalImage = std::make_shared<Image>(m_Device, width, height, VK_FORMAT_R8G8B8A8_UNORM);
		}
	}
	void GpuPathTracer::RayTrace(const VkCommandBuffer& cmdBuffer, uint32_t imageIndex)
	{
		auto& pipeline_properties = m_Pipeline.GetPipelineProperties();
		const uint32_t handle_size_aligned = m_Pipeline.aligned_size(pipeline_properties.shaderGroupHandleSize, pipeline_properties.shaderGroupHandleAlignment);

		VkStridedDeviceAddressRegionKHR raygen_shader_sbt_entry{};
		raygen_shader_sbt_entry.deviceAddress = m_Device.get_buffer_device_address(m_Pipeline.GetRayGenSBT().Get());
		raygen_shader_sbt_entry.stride = handle_size_aligned;
		raygen_shader_sbt_entry.size = handle_size_aligned;

		VkStridedDeviceAddressRegionKHR miss_shader_sbt_entry{};
		miss_shader_sbt_entry.deviceAddress = m_Device.get_buffer_device_address(m_Pipeline.GetMissSBT().Get());
		miss_shader_sbt_entry.stride = handle_size_aligned;
		miss_shader_sbt_entry.size = handle_size_aligned;

		VkStridedDeviceAddressRegionKHR hit_shader_sbt_entry{};
		hit_shader_sbt_entry.deviceAddress = m_Device.get_buffer_device_address(m_Pipeline.GetHitSBT().Get());
		hit_shader_sbt_entry.stride = handle_size_aligned;
		hit_shader_sbt_entry.size = handle_size_aligned;

		VkStridedDeviceAddressRegionKHR callable_shader_sbt_entry{};

		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_Pipeline.Get());
		vkCmdBindDescriptorSets(cmdBuffer, 
			VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, 
			m_Pipeline.GetLayout(),
			0, 1,
			m_DescriptorSets.data(),
			0, nullptr);

		pfn_vkCmdTraceRaysKHR(cmdBuffer,
			&raygen_shader_sbt_entry,
			&miss_shader_sbt_entry,
			&hit_shader_sbt_entry,
			&callable_shader_sbt_entry,
			m_FinalImage->GetWidth(), m_FinalImage->GetHeight(), 1);

		UpdateUniformBuffer(m_CurrentFrame);
	}

	void GpuPathTracer::CreateDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 6> pool_sizes;

		pool_sizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		pool_sizes[0].descriptorCount = 1;

		pool_sizes[1].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		pool_sizes[1].descriptorCount = 1;

		pool_sizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_sizes[2].descriptorCount = 1;

		pool_sizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		pool_sizes[3].descriptorCount = 1;

		pool_sizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		pool_sizes[4].descriptorCount = 1;

		pool_sizes[5].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		pool_sizes[5].descriptorCount = 1;

		VkDescriptorPoolCreateInfo pool_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		pool_info.maxSets = 1;
		pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		pool_info.pPoolSizes = pool_sizes.data();

		VK_CHECK(vkCreateDescriptorPool(
			m_Device.Get(),
			&pool_info,
			m_Device.GetAllocator(),
			&m_DescriptorPool));

		std::cout << "[VULKAN] RT Descriptor pool created!\n";
	}
	void GpuPathTracer::CreateDescriptorSetLayout()
	{
		std::array<VkDescriptorSetLayoutBinding, 6> bindings;

		VkDescriptorSetLayoutBinding acceleration_structure_layout_binding{};
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

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

		VkDescriptorSetLayoutBinding vertex_buffer_binding{};
		bindings[3].binding = 3;
		bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bindings[3].descriptorCount = 1;
		bindings[3].stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

		VkDescriptorSetLayoutBinding index_buffer_binding{};
		bindings[4].binding = 4;
		bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bindings[4].descriptorCount = 1;
		bindings[4].stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

		VkDescriptorSetLayoutBinding obj_descs_buffer_binding{};
		bindings[5].binding = 5;
		bindings[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bindings[5].descriptorCount = 1;
		bindings[5].stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

		VkDescriptorSetLayoutCreateInfo layout_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
		layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
		layout_info.pBindings = bindings.data();

		VK_CHECK(vkCreateDescriptorSetLayout(
			m_Device.Get(),
			&layout_info,
			m_Device.GetAllocator(),
			&m_DescriptorSetLayout));

		std::cout << "[VULKAN] RT Descriptor set layout created!\n";
	}
	void GpuPathTracer::CreateDescriptorSets()
	{
		std::array<VkDescriptorSetLayout, 1> layouts = { m_DescriptorSetLayout };

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = layouts.data();

		m_DescriptorSets.resize(1);

		VK_CHECK(vkAllocateDescriptorSets(m_Device.Get(), &allocInfo, &m_DescriptorSets[0]));

		// Setup the descriptor for binding our top level acceleration structure to the ray tracing shaders
		VkWriteDescriptorSetAccelerationStructureKHR descriptor_acceleration_structure_info{};
		descriptor_acceleration_structure_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		descriptor_acceleration_structure_info.accelerationStructureCount = 1;
		descriptor_acceleration_structure_info.pAccelerationStructures = &m_Scene->GetTLAS();

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

		VkWriteDescriptorSet result_image_write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		result_image_write.dstSet = m_DescriptorSets[0];
		result_image_write.dstBinding = 1;
		result_image_write.descriptorCount = 1;
		result_image_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		result_image_write.pImageInfo = &image_descriptor;

		VkDescriptorBufferInfo buffer_descriptor{};
		buffer_descriptor.buffer = m_UniformBuffers[0].Get();
		buffer_descriptor.offset = 0;
		buffer_descriptor.range = sizeof(RTUniformBufferObject);

		VkWriteDescriptorSet uniform_buffer_write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		uniform_buffer_write.dstSet = m_DescriptorSets[0];
		uniform_buffer_write.dstBinding = 2;
		uniform_buffer_write.descriptorCount = 1;
		uniform_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniform_buffer_write.pBufferInfo = &buffer_descriptor;

		VkDescriptorBufferInfo vertex_buffer_descriptor{};
		vertex_buffer_descriptor.buffer = m_Scene->GetVertexBuffer().Get();
		vertex_buffer_descriptor.offset = 0;
		vertex_buffer_descriptor.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet vertex_buffer_write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		vertex_buffer_write.dstSet = m_DescriptorSets[0];
		vertex_buffer_write.dstBinding = 3;
		vertex_buffer_write.descriptorCount = 1;
		vertex_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		vertex_buffer_write.pBufferInfo = &vertex_buffer_descriptor;

		VkDescriptorBufferInfo index_buffer_descriptor{};
		index_buffer_descriptor.buffer = m_Scene->GetIndexBuffer().Get();
		index_buffer_descriptor.offset = 0;
		index_buffer_descriptor.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet index_buffer_write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		index_buffer_write.dstSet = m_DescriptorSets[0];
		index_buffer_write.dstBinding = 4;
		index_buffer_write.descriptorCount = 1;
		index_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		index_buffer_write.pBufferInfo = &index_buffer_descriptor;

		VkDescriptorBufferInfo obj_descs_buffer_descriptor{};
		obj_descs_buffer_descriptor.buffer = m_Scene->GetObjDescsBuffer().Get();
		obj_descs_buffer_descriptor.offset = 0;
		obj_descs_buffer_descriptor.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet obj_descs_buffer_write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		obj_descs_buffer_write.dstSet = m_DescriptorSets[0];
		obj_descs_buffer_write.dstBinding = 5;
		obj_descs_buffer_write.descriptorCount = 1;
		obj_descs_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		obj_descs_buffer_write.pBufferInfo = &obj_descs_buffer_descriptor;

		std::vector<VkWriteDescriptorSet> write_descriptor_sets = {
			acceleration_structure_write,
			result_image_write,
			uniform_buffer_write,
			vertex_buffer_write,
			index_buffer_write,
			obj_descs_buffer_write };
		vkUpdateDescriptorSets(m_Device.Get(),
			static_cast<uint32_t>(write_descriptor_sets.size()), 
			write_descriptor_sets.data(),
			0, VK_NULL_HANDLE);
	}

	void GpuPathTracer::UpdateStorageImageDescriptor() const
	{
		VkDescriptorImageInfo image_descriptor{};
		image_descriptor.imageView = m_FinalImage->GetImageView();
		image_descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkWriteDescriptorSet result_image_write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		result_image_write.dstSet = m_DescriptorSets[0];
		result_image_write.dstBinding = 1;
		result_image_write.descriptorCount = 1;
		result_image_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		result_image_write.pImageInfo = &image_descriptor;

		vkUpdateDescriptorSets(m_Device.Get(),
			1,
			&result_image_write,
			0, VK_NULL_HANDLE);
	}

	void GpuPathTracer::UpdateUniformBuffer(uint32_t currentImage)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		memcpy(m_UniformBuffers[0].GetMappedBuffer(), &RTubo, sizeof(RTubo));

		RTubo.view = m_Camera->GetView().GetInverse();
		RTubo.proj = m_Camera->GetProjection().GetInverse();
	}
}