#pragma once
#include <vector>

#include "HittableList.h"
#include "Mesh.h"
#include "Math/Vector3.h"
#include "Triangle.h"

namespace LightBox
{
	struct ObjDesc
	{
		uint32_t index_offset;
		uint32_t primitive_count;
	};

	struct Vertex2
	{
		Vector3 pos;
		Vector3 nrm;
		Vector2 uv;
	};

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

		void CreateInterleavedVertexBuffer();

		void LoadEnvMap(const std::string& path);

		HittableList& GetHittableList() { return m_Hittables; }
		const std::vector<Vertex2>& GetVertexBuffer() { return m_VertexBuffer; }
		const std::vector<uint32_t>& GetIndexBuffer() { return m_IndexUV; }
		const std::vector<ObjDesc>& GetObjDescriptions() { return m_ObjDescs; }

		std::vector<Mesh*>& GetMeshes() { return m_Meshes; }

	public:
		HittableList m_Hittables;
		std::vector<Mesh*> m_Meshes;
		std::vector<Vector3> m_Vertices;
		std::vector<Vector3> m_Normals;
		std::vector<Vector2> m_VertexUvs;
		std::vector<uint32_t> m_Indices;
		std::vector<uint32_t> m_UvsIndices;

		// For interleaved vertex attributes
		std::vector<Vertex2> m_VertexBuffer;
		std::vector<uint32_t> m_IndexUV;
		std::vector<uint32_t> m_IndexNrm;
		std::vector<uint32_t> m_IndexPos;
		std::vector<Vector2> m_Uvs;

		std::vector<ObjDesc> m_ObjDescs;

		Lambertian m_DefaultMaterial = Lambertian(Vector3(0.8f));
		std::vector<Texture*> m_Textures;
		std::vector<Material*> m_Materials;

		ImageTexture* m_EnvMap;
	};
}

