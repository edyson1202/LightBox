#include "VulkanScene.h"

#include "Vertex.h"

namespace LightBox
{
	VulkanScene::VulkanScene(Device& device)
		: m_Device(device), m_VertexBuffer(m_Device), m_IndexBuffer(m_Device)
	{
	}

	void VulkanScene::LoadSceneFromRAM(const Scene& scene, 
		std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
	{

		m_VertexCount = scene.m_Vertices.size();
		m_IndicesCount = scene.m_Indices.size();

		// creating a Vertex vector with interleaved Vertex attributes
		std::vector<Vertex> verts(m_VertexCount + vertices.size());
		for (int32_t i = 0; i < m_VertexCount; i++)
		{
			verts[i].pos = scene.m_Vertices[i];
			verts[i].col = { 1.f, 1.f, 0.f };
		}
		for (int32_t i = 0; i < vertices.size(); i++)
		{
			verts[m_VertexCount + i] = vertices[i];
		}
		std::vector<uint32_t> gpu_indices(m_IndicesCount + indices.size());
		for (int32_t i = 0; i < m_IndicesCount; i++)
		{
			gpu_indices[i] = scene.m_Indices[i];
		}
		for (int32_t i = 0; i < indices.size(); i++)
		{
			gpu_indices[m_IndicesCount + i] = m_VertexCount + indices[i];
		}
		m_IndicesCount += indices.size();

		m_VertexBuffer.CreateVertexBuffer(verts);
		m_IndexBuffer.CreateIndexBuffer(gpu_indices);

		vkDeviceWaitIdle(m_Device.GetDevice());
	}
}
