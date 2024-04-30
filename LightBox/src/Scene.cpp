#include "Scene.h"

#include "Vertex.h"
#include "Vector3.h"
#include "VulkanUtils.h"

namespace LightBox
{
	//uint64_t Scene::get_buffer_device_address(VkBuffer buffer)
	//{
	//	VkBufferDeviceAddressInfoKHR buffer_device_address_info{};
	//	buffer_device_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	//	buffer_device_address_info.buffer = buffer;
	//	return m_Device.vkGetBufferDeviceAddressKHR(m_Device.GetDevice(), &buffer_device_address_info);
	//}
	std::vector<Vertex> obj_vertices_old = {
	{{-1, -1, 1}, { 1, 0, 0 }}, {{-1, 1, 1}, { 0, 1, 0 }},
	{{1, 1, 1}, { 0, 0, 1 }} , {{1, -1, 1}, { 1, 1, 0 }},
	{{-1, -1, -1}, { 1, 1, 1 }}, {{-1, 1, -1}, { 1, 1, 1 }},
	{{1, 1, -1}, { 1, 1, 1 }} , {{1, -1, -1}, { 1, 1, 1 }}
	};
	std::vector<Vector3> obj_vertices = {
	{-1, -1, 1}, {-1, 1, 1}, {1, 1, 1} , {1, -1, 1},
	{-1, -1, -1}, {-1, 1, -1}, {1, 1, -1} , {1, -1, -1}
	};
	std::vector<uint32_t> obj_indices = { 0, 1, 2, 2, 3, 0,
										   7, 6, 5, 5, 4, 7,
										   3, 2, 6, 6, 7, 3,
										   4, 5, 1, 1, 0, 4,
										   4, 0, 3, 3, 7, 4,
										   1, 5, 6, 6, 2, 1 };
	uint32_t vertices_size = obj_vertices.size() * sizeof(Vector3);
	uint32_t indices_size = obj_indices.size() * sizeof(uint32_t);

	Scene::Scene(Device& device) 
		: m_Device(device), m_BLASBuffer(device), m_VertexBuffer(m_Device), m_IndexBuffer(m_Device)
	{
		UploadSceneToGPU();
		CreateAccelerationStructure();

		//CreateDescriptorPool();
		//CreateDescriptorSetLayout();
		//CreateDescriptorSets();
	}
	Scene::~Scene()
	{
		vkDestroyDescriptorSetLayout(m_Device.GetDevice(), m_DescriptorSetLayout,
			m_Device.GetAllocator());
		vkDestroyDescriptorPool(m_Device.GetDevice(), m_DescriptorPool,
			m_Device.GetAllocator());
	}
	void Scene::UploadSceneToGPU()
	{
		const VkBufferUsageFlags buffer_usage_flags = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

		m_VertexBuffer.CreateBuffer(vertices_size, buffer_usage_flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_VertexBuffer.Update(obj_vertices.data(), vertices_size);
		m_IndexBuffer.CreateBuffer(indices_size, buffer_usage_flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_IndexBuffer.Update(obj_indices.data(), indices_size);
	}
	void Scene::CreateAccelerationStructure()
	{
		{
			VkDeviceOrHostAddressConstKHR vertex_data_device_address{};
			VkDeviceOrHostAddressConstKHR index_data_device_address{};

			vertex_data_device_address.deviceAddress = m_Device.get_buffer_device_address(m_VertexBuffer.GetBuffer());
			index_data_device_address.deviceAddress = m_Device.get_buffer_device_address(m_IndexBuffer.GetBuffer());

			VkAccelerationStructureGeometryTrianglesDataKHR triangles{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR };
			triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
			triangles.vertexData = vertex_data_device_address;
			triangles.vertexStride = 3 * sizeof(float);
			triangles.indexType = VK_INDEX_TYPE_UINT32;
			triangles.indexData = index_data_device_address;
			triangles.maxVertex = uint32_t(obj_vertices.size() - 1);
			triangles.transformData = { 0 }; // No transform

			VkAccelerationStructureGeometryKHR geometry{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
			geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
			geometry.geometry.triangles = triangles;
			geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

			VkAccelerationStructureBuildRangeInfoKHR range_info;
			range_info.firstVertex = 0;
			range_info.primitiveCount = uint32_t(obj_indices.size() / 3);
			range_info.primitiveOffset = 0;
			range_info.transformOffset = 0;

			VkAccelerationStructureBuildGeometryInfoKHR build_info{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
			build_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
			build_info.geometryCount = 1;
			build_info.pGeometries = &geometry;
			build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
			build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			build_info.srcAccelerationStructure = VK_NULL_HANDLE;

			VkAccelerationStructureBuildSizesInfoKHR sizeInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
			m_Device.vkGetAccelerationStructureBuildSizesKHR(m_Device.GetDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
				&build_info, &range_info.primitiveCount, &sizeInfo);

			VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR
				| VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
				| VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			m_BLASBuffer.CreateBuffer(sizeInfo.accelerationStructureSize, usage_flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			// Create an empty acceleration structure object.
			VkAccelerationStructureCreateInfoKHR create_info{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
			create_info.type = build_info.type;
			create_info.size = sizeInfo.accelerationStructureSize;
			create_info.buffer = m_BLASBuffer.GetBuffer();
			create_info.offset = 0;
			VkResult res = m_Device.vkCreateAccelerationStructureKHR(m_Device.GetDevice(), &create_info, nullptr, &m_Blas);
			check_vk_result(res);
			build_info.dstAccelerationStructure = m_Blas;

			m_BLASScratchBuffer.CreateBuffer(sizeInfo.buildScratchSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
				| VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			build_info.scratchData.deviceAddress = m_Device.get_buffer_device_address(m_BLASScratchBuffer.GetBuffer());

			VkCommandBuffer cmd_buffer = m_Device.BeginSingleTimeCommands();
			VkAccelerationStructureBuildRangeInfoKHR* pRangeInfo = &range_info;
			m_Device.vkCmdBuildAccelerationStructuresKHR(cmd_buffer, 1, &build_info, &pRangeInfo);
			m_Device.EndSingleTimeCommands(cmd_buffer);
		}

		// TLAS
		
		VkAccelerationStructureDeviceAddressInfoKHR addressInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
		addressInfo.accelerationStructure = m_Blas;
		VkDeviceAddress blasAddress = vkGetAccelerationStructureDeviceAddressKHR(m_Device.GetDevice(),
			&addressInfo);

		// Zero -initialize.
		VkAccelerationStructureInstanceKHR instance{};
		// Set the instance transform to a 135- degree rotation around
		// the y-axis.
		const float rcpSqrt2 = sqrtf(0.5f);
		instance.transform.matrix[0][0] = -rcpSqrt2;
		instance.transform.matrix[0][2] = rcpSqrt2;
		instance.transform.matrix[1][1] = 1.0f;
		instance.transform.matrix[2][0] = -rcpSqrt2;
		instance.transform.matrix[2][2] = -rcpSqrt2;
		instance.instanceCustomIndex = 0;
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		instance.accelerationStructureReference = blasAddress;

		VkCommandBuffer cmd_buffer = m_Device.BeginSingleTimeCommands();
		m_InstancesBuffer.CreateBuffer(sizeof(instance), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
			| VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_InstancesBuffer.Update(&instance, sizeof(instance));
		
		// Add a memory barrier to ensure that createBuffer 's upload command
		// finishes before starting the TLAS build.
		VkMemoryBarrier barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
		vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, 
			VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);

		m_Device.EndSingleTimeCommands(cmd_buffer);

		VkAccelerationStructureBuildRangeInfoKHR rangeInfo;
		rangeInfo.primitiveOffset = 0;
		rangeInfo.primitiveCount = 1; // Number of instances
		rangeInfo.firstVertex = 0;
		rangeInfo.transformOffset = 0;

		VkAccelerationStructureGeometryInstancesDataKHR instancesVk{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR };
		instancesVk.arrayOfPointers = VK_FALSE;
		instancesVk.data.deviceAddress = m_Device.get_buffer_device_address(m_InstancesBuffer.GetBuffer());
		// Like creating the BLAS , point to the geometry (in this case , the
		// instances) in a polymorphic object.
		VkAccelerationStructureGeometryKHR geometry{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
		geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		geometry.geometry.instances = instancesVk;


		// Create the build info: in this case , pointing to only one
		// geometry object.
		VkAccelerationStructureBuildGeometryInfoKHR buildInfo{
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
		buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		buildInfo.geometryCount = 1;
		buildInfo.pGeometries = &geometry;
		buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;
		
		// Query the worst -case AS size and scratch space size based on
		// the number of instances (in this case , 1).
		VkAccelerationStructureBuildSizesInfoKHR sizeInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
		m_Device.vkGetAccelerationStructureBuildSizesKHR(m_Device.GetDevice(), 
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, & buildInfo, & rangeInfo.primitiveCount,
			& sizeInfo);
		
		// Allocate a buffer for the acceleration structure.
		m_TLASBuffer.CreateBuffer(sizeInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR
			| VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		
		// Create the acceleration structure object.
		// (Data has not yet been set.)
		VkAccelerationStructureCreateInfoKHR createInfo{
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
		createInfo.type = buildInfo.type;
		createInfo.size = sizeInfo.accelerationStructureSize;
		createInfo.buffer = m_TLASBuffer.GetBuffer();
		createInfo.offset = 0;
		VkResult res = (m_Device.vkCreateAccelerationStructureKHR(m_Device.GetDevice(), &createInfo, nullptr, &m_TLAS));
		check_vk_result(res);
		
		buildInfo.dstAccelerationStructure = m_TLAS;
		
		// Allocate the scratch buffer holding temporary build data.
		m_TLASScratchBuffer.CreateBuffer(sizeInfo.buildScratchSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
			| VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		buildInfo.scratchData.deviceAddress = m_Device.get_buffer_device_address(m_TLASScratchBuffer.GetBuffer());
		
		// Create a one -element array of pointers to range info objects.
		VkAccelerationStructureBuildRangeInfoKHR * pRangeInfo = &rangeInfo;	
		VkCommandBuffer cmd = m_Device.BeginSingleTimeCommands();
		// Build the TLAS.
		m_Device.vkCmdBuildAccelerationStructuresKHR(cmd, 1, &buildInfo, &pRangeInfo);

		m_Device.EndSingleTimeCommands(cmd);
	}
	void Scene::CreateDescriptorPool()
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
	void Scene::CreateDescriptorSetLayout()
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
	//void Scene::CreateDescriptorSets()
	//{
	//	std::vector<VkDescriptorSetLayout> layouts(1, m_DescriptorSetLayout);

	//	VkDescriptorSetAllocateInfo allocInfo{};
	//	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	//	allocInfo.descriptorPool = m_DescriptorPool;
	//	allocInfo.descriptorSetCount = 1;
	//	allocInfo.pSetLayouts = layouts.data();

	//	VkResult res = vkAllocateDescriptorSets(m_Device.GetDevice(), &allocInfo, &m_DescriptorSets);
	//	check_vk_result(res);

	//	// Setup the descriptor for binding our top level acceleration structure to the ray tracing shaders
	//	VkWriteDescriptorSetAccelerationStructureKHR descriptor_acceleration_structure_info{};
	//	descriptor_acceleration_structure_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	//	descriptor_acceleration_structure_info.accelerationStructureCount = 1;
	//	descriptor_acceleration_structure_info.pAccelerationStructures = &m_TLAS;

	//	VkWriteDescriptorSet acceleration_structure_write{};
	//	acceleration_structure_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	//	acceleration_structure_write.dstSet = m_DescriptorSets;
	//	acceleration_structure_write.dstBinding = 0;
	//	acceleration_structure_write.descriptorCount = 1;
	//	acceleration_structure_write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	//	// The acceleration structure descriptor has to be chained via pNext
	//	acceleration_structure_write.pNext = &descriptor_acceleration_structure_info;

	//	VkDescriptorImageInfo image_descriptor{};
	//	image_descriptor.imageView = storage_image.view;
	//	image_descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	//	VkDescriptorBufferInfo buffer_descriptor = create_descriptor(*ubo);
	//	VkDescriptorBufferInfo bufferInfo{};
	//	bufferInfo.buffer = m_UniformBuffers[i].GetBuffer();
	//	bufferInfo.offset = 0;
	//	bufferInfo.range = sizeof(UniformBufferObject);

	//	VkWriteDescriptorSet result_image_write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	//	result_image_write.dstSet = m_DescriptorSets;
	//	result_image_write.dstBinding = 1;
	//	result_image_write.descriptorCount = 1;
	//	result_image_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	//	result_image_write.pImageInfo = &image_descriptor;

	//	VkWriteDescriptorSet uniform_buffer_write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	//	uniform_buffer_write.dstSet = m_DescriptorSets;
	//	uniform_buffer_write.dstBinding = 2;
	//	uniform_buffer_write.descriptorCount = 1;
	//	uniform_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	//	uniform_buffer_write.pBufferInfo = &buffer_descriptor;

	//	std::vector<VkWriteDescriptorSet> write_descriptor_sets = {
	//		acceleration_structure_write,
	//		result_image_write,
	//		uniform_buffer_write };
	//	vkUpdateDescriptorSets(m_Device.GetDevice(), static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, VK_NULL_HANDLE);
	//}
}