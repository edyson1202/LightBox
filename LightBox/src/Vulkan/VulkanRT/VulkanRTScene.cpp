#include "VulkanRTScene.h"

#include "Vertex.h"
#include "Math/Vector3.h"
#include "Vulkan/VulkanUtils.h"

namespace LightBox
{
	VulkanRTScene::VulkanRTScene(Device& device)
		: m_Device(device) {}

	// TODO pass the Scene and store it as a reference instead of multiple buffers
	void VulkanRTScene::UploadSceneToGPU(const std::vector<Vertex2>& vertex_buffer, const std::vector<uint32_t>& index_buffer, const std::vector<ObjDesc>& obj_descs)
	{
		const VkBufferUsageFlags buffer_usage_flags = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR
		| VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
		| VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

		uint32_t m_VertexBufferSize = vertex_buffer.size() * sizeof(Vertex2);
		uint32_t m_IndexBufferSize = index_buffer.size() * sizeof(uint32_t);
		uint32_t m_ObjDescsBufferSize = obj_descs.size() * sizeof(ObjDesc);

		m_VertexBuffer.CreateBuffer(m_VertexBufferSize, buffer_usage_flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_VertexBuffer.Update((void*)vertex_buffer.data(), m_VertexBufferSize);
		m_IndexBuffer.CreateBuffer(m_IndexBufferSize, buffer_usage_flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_IndexBuffer.Update((void*)index_buffer.data(), m_IndexBufferSize);

		m_ObjDescsBuffer.CreateBuffer(m_ObjDescsBufferSize,
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_ObjDescsBuffer.Update((void*)obj_descs.data(), m_ObjDescsBufferSize);

		vertex_data_device_address.deviceAddress = m_Device.get_buffer_device_address(m_VertexBuffer.Get());
		index_data_device_address.deviceAddress = m_Device.get_buffer_device_address(m_IndexBuffer.Get());

		CreateAccelerationStructure(vertex_buffer, index_buffer, obj_descs);
	}
	void VulkanRTScene::CreateAccelerationStructure(const std::vector<Vertex2>& vertex_buffer, const std::vector<uint32_t>& index_buffer, const std::vector<ObjDesc>& obj_descs)
	{
		VkCommandBuffer cmd_buffer1 = m_Device.BeginSingleTimeCommands();

		m_BLASBuffers.resize(obj_descs.size());
		m_BLASScratchBuffers.resize(obj_descs.size());
		m_BlAccelerationStructures.resize(obj_descs.size());

		for (int32_t i = 0; i < obj_descs.size(); i++)
			BuildBottomLevelAS(cmd_buffer1, m_BLASBuffers[i], m_BLASScratchBuffers[i], obj_descs[i], m_BlAccelerationStructures[i]);

		m_Device.EndSingleTimeCommands(cmd_buffer1);

		// TLAS

		std::vector<VkAccelerationStructureInstanceKHR> instances(obj_descs.size());

		for (int32_t i = 0; i < obj_descs.size(); i++)
		{
			VkAccelerationStructureDeviceAddressInfoKHR addressInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
			addressInfo.accelerationStructure = m_BlAccelerationStructures[i];
			VkDeviceAddress blasAddress = pfn_vkGetAccelerationStructureDeviceAddressKHR(m_Device.Get(), &addressInfo);

			// Zero -initialize.
			VkAccelerationStructureInstanceKHR& instance = instances[i];
			// Identity transform with translation
			instance.transform.matrix[0][0] = 1;
			instance.transform.matrix[1][1] = 1;
			instance.transform.matrix[2][2] = 1;
			instance.transform.matrix[3][3] = 1;

			instance.transform.matrix[3][2] = -8;

			instance.instanceCustomIndex = i;
			instance.mask = 0xFF;
			instance.instanceShaderBindingTableRecordOffset = 0;
			instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
			instance.accelerationStructureReference = blasAddress;
		}

		VkCommandBuffer cmd_buffer = m_Device.BeginSingleTimeCommands();
		m_InstancesBuffer.CreateBuffer(sizeof(VkAccelerationStructureInstanceKHR) * instances.size(),
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_InstancesBuffer.Update(instances.data(), sizeof(VkAccelerationStructureInstanceKHR) * instances.size());
		
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
		rangeInfo.primitiveCount = static_cast<uint32_t>(instances.size()); // Number of instances
		rangeInfo.firstVertex = 0;
		rangeInfo.transformOffset = 0;

		VkAccelerationStructureGeometryInstancesDataKHR instancesVk{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR };
		instancesVk.arrayOfPointers = VK_FALSE;
		instancesVk.data.deviceAddress = m_Device.get_buffer_device_address(m_InstancesBuffer.Get());
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
		pfn_vkGetAccelerationStructureBuildSizesKHR(
			m_Device.Get(), 
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&buildInfo,
			&rangeInfo.primitiveCount,
			&sizeInfo);
		
		// Allocate a buffer for the acceleration structure.
		m_TLASBuffer.CreateBuffer(
			sizeInfo.accelerationStructureSize,
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR
			| VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
			| VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		
		// Create the acceleration structure object.
		// (Data has not yet been set.)
		VkAccelerationStructureCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
		createInfo.type = buildInfo.type;
		createInfo.size = sizeInfo.accelerationStructureSize;
		createInfo.buffer = m_TLASBuffer.Get();
		createInfo.offset = 0;
		VkResult res = (pfn_vkCreateAccelerationStructureKHR(m_Device.Get(), &createInfo, nullptr, &m_TLAS));
		check_vk_result(res);
		
		buildInfo.dstAccelerationStructure = m_TLAS;
		
		// Allocate the scratch buffer holding temporary build data.
		m_TLASScratchBuffer.CreateBuffer(
			sizeInfo.buildScratchSize,
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
			| VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		buildInfo.scratchData.deviceAddress = m_Device.get_buffer_device_address(m_TLASScratchBuffer.Get());
		
		// Create a one -element array of pointers to range info objects.
		VkAccelerationStructureBuildRangeInfoKHR *pRangeInfo = &rangeInfo;	
		VkCommandBuffer cmd = m_Device.BeginSingleTimeCommands();
		// Build the TLAS.
		pfn_vkCmdBuildAccelerationStructuresKHR(cmd, 1, &buildInfo, &pRangeInfo);

		m_Device.EndSingleTimeCommands(cmd);
	}

	void VulkanRTScene::BuildBottomLevelAS(VkCommandBuffer cmd_buffer, Buffer& buffer,
		Buffer& scratch_buffer, const ObjDesc& obj_desc, VkAccelerationStructureKHR& blas)
	{
		VkAccelerationStructureGeometryTrianglesDataKHR triangles{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR };
		triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		triangles.vertexData = vertex_data_device_address;
		triangles.vertexStride = sizeof(Vertex2);
		triangles.indexType = VK_INDEX_TYPE_UINT32;
		triangles.indexData = index_data_device_address;
		// TODO set this to an appropriate value
		triangles.maxVertex = 0;
		triangles.transformData = { 0 }; // No transform

		VkAccelerationStructureGeometryKHR geometry{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
		geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		geometry.geometry.triangles = triangles;
		geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

		VkAccelerationStructureBuildRangeInfoKHR range_info;
		range_info.firstVertex = 0;
		range_info.primitiveCount = obj_desc.primitive_count;
		// offset in bytes in the index buffer
		range_info.primitiveOffset = obj_desc.index_offset * sizeof(uint32_t);
		range_info.transformOffset = 0;

		VkAccelerationStructureBuildGeometryInfoKHR build_info{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
		build_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		build_info.geometryCount = 1;
		build_info.pGeometries = &geometry;
		build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		build_info.srcAccelerationStructure = VK_NULL_HANDLE;

		VkAccelerationStructureBuildSizesInfoKHR sizeInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };

		pfn_vkGetAccelerationStructureBuildSizesKHR(
			m_Device.Get(),
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&build_info,
			&range_info.primitiveCount,
			&sizeInfo);

		VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR
			| VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
			| VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		buffer.CreateBuffer(sizeInfo.accelerationStructureSize, usage_flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// Create an empty acceleration structure object.
		VkAccelerationStructureCreateInfoKHR create_info{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
		create_info.type = build_info.type;
		create_info.size = sizeInfo.accelerationStructureSize;
		create_info.buffer = buffer.Get();
		create_info.offset = 0;

		VkResult res = pfn_vkCreateAccelerationStructureKHR(m_Device.Get(), &create_info, nullptr, &blas);
		check_vk_result(res);

		build_info.dstAccelerationStructure = blas;

		scratch_buffer.CreateBuffer(sizeInfo.buildScratchSize,
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		build_info.scratchData.deviceAddress = m_Device.get_buffer_device_address(scratch_buffer.Get());

		VkAccelerationStructureBuildRangeInfoKHR* pRangeInfo = &range_info;
		pfn_vkCmdBuildAccelerationStructuresKHR(cmd_buffer, 1, &build_info, &pRangeInfo);
	}
}
