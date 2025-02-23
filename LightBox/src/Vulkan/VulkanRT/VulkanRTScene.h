#pragma once

#include "Scene.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/Device.h"

namespace LightBox
{
	class VulkanRTScene
	{
	public:
		VulkanRTScene(Device& device);
		~VulkanRTScene() = default;

		VkAccelerationStructureKHR& GetTLAS() { return m_TLAS; }
		Buffer& GetVertexBuffer() { return m_VertexBuffer; }
		Buffer& GetIndexBuffer() { return m_IndexBuffer; }
		Buffer& GetObjDescsBuffer() { return m_ObjDescsBuffer; }

		void UploadSceneToGPU(const std::vector<Vertex2>& vertex_buffer, const std::vector<uint32_t>& index_buffer, const std::vector<ObjDesc>& obj_descs);

	private:
		void CreateAccelerationStructure(const std::vector<Vertex2>& vertex_buffer, const std::vector<uint32_t>& index_buffer, const std::vector<ObjDesc>& obj_descs);

		void BuildBottomLevelAS(VkCommandBuffer cmd_buffer, Buffer& buffer, Buffer& scratch_buffer, const ObjDesc& obj_desc, VkAccelerationStructureKHR& blas);

	private:
		Device& m_Device;

		Buffer m_VertexBuffer;
		Buffer m_IndexBuffer;
		Buffer m_ObjDescsBuffer;

		VkAccelerationStructureKHR m_TLAS = VK_NULL_HANDLE;
		Buffer m_TLASBuffer;
		Buffer m_TLASScratchBuffer;

		Buffer m_InstancesBuffer;

		std::vector<VkAccelerationStructureKHR> m_BlAccelerationStructures;
		std::vector<Buffer> m_BLASBuffers;
		std::vector<Buffer> m_BLASScratchBuffers;

		VkDeviceOrHostAddressConstKHR vertex_data_device_address{};
		VkDeviceOrHostAddressConstKHR index_data_device_address{};
	};
}
