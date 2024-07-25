#pragma once
#include <vector>

#include "Math/Vector3.h"
#include "Triangle.h"

namespace LightBox
{
	class Scene
	{
	public:
		Scene() = default;

		std::vector<Triangle> Scene::LoadDataFromOBJ(const std::string& path);

	public:
		std::vector<Vector3> m_Vertices;
		std::vector<uint32_t> m_Indices;
	};
}

