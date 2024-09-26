#pragma once
#include <vector>

#include "HittableList.h"
#include "Mesh.h"
#include "Math/Vector3.h"
#include "Triangle.h"

namespace LightBox
{
	class Scene
	{
	public:
		Scene() = default;
		~Scene()
		{
			for (const Texture* tex : m_Textures)
				delete tex;
			for (const Material* mat : m_Materials)
				delete mat;
		}

		void LoadDataFromObj(const std::string& path);

		void LoadEnvMap(const std::string& path);

		HittableList& GetHittableList() { return m_Hittables; }

		std::vector<Mesh*>& GetMeshes() { return m_Meshes; }

	public:
		HittableList m_Hittables;
		std::vector<Mesh*> m_Meshes;
		std::vector<Vector3> m_Vertices;
		std::vector<Vector2> m_VertexUvs;
		std::vector<uint32_t> m_Indices;
		std::vector<uint32_t> m_UvsIndices;

		Lambertian m_DefaultMaterial = Lambertian(Vector3(0.8f));
		std::vector<Texture*> m_Textures;
		std::vector<Material*> m_Materials;

		ImageTexture* m_EnvMap;
	};
}

