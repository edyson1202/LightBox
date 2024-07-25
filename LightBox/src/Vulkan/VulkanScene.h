#pragma once
#include "Buffer.h"
#include "Scene.h"

namespace LightBox
{
	class VulkanScene
	{
	public:
		VulkanScene(Device& device);

		void LoadSceneFromRAM(const Scene& scene);


	private:
		Device& m_Device;
	public:
		int32_t m_VertexCount;
		int32_t m_IndicesCount;
		Buffer m_VertexBuffer;
		Buffer m_IndexBuffer;

	};
}

