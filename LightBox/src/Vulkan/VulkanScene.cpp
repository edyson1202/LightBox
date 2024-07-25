#include "VulkanScene.h"

#include "Vertex.h"

namespace LightBox
{
	VulkanScene::VulkanScene(Device& device)
		: m_Device(device), m_VertexBuffer(m_Device), m_IndexBuffer(m_Device)
	{
	}

	void VulkanScene::LoadSceneFromRAM(const Scene& scene)
	{
		m_VertexCount = scene.m_Vertices.size();
		m_IndicesCount = scene.m_Indices.size();

		// creating a Vertex vector with interleaved Vertex attributes
		std::vector<Vertex> vertices(m_VertexCount);
		for (int32_t i = 0; i < m_VertexCount; i++)
		{
			vertices[i].pos = scene.m_Vertices[i];
			vertices[i].col = { 1.f, 1.f, 0.f };
		}

		m_VertexBuffer.CreateVertexBuffer(vertices);
		m_IndexBuffer.CreateIndexBuffer(scene.m_Indices);
	}
}
