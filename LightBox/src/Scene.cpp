#include "Scene.h"

#include <string>
#include <unordered_set>

#include "Sphere.h"
#include "Timer.h"
#include "Vulkan/Image.h"

namespace LightBox
{

	void Scene::LoadDataFromObj(const std::string& path)
	{
		Timer timer;

		std::string line;
		std::ifstream file(path);

		std::vector<std::string> mesh_names;

		while (std::getline(file, line)) {
			if (line[0] == 'v' && line[1] == ' ')
			{
				float values[3];
				int32_t token_start_index = 2;
				int32_t axis_index = 0;
				for (int32_t i = 2; i < line.length() + 1; i++)
				{
					if (line[i] == ' ' || line[i] == 0)
					{
						values[axis_index] = std::stof(line.substr(token_start_index, i - token_start_index));

						token_start_index = i + 1;
						axis_index++;
					}
				}
				m_Vertices.emplace_back(values[0], values[1], values[2]);
			}
			else if (line[0] == 'v' && line[1] == 't')
			{
				std::string string_values[2];
				int string_index = 0;

				for (int32_t i = 3; i < line.length(); i++) {
					if (line[i] == ' ')
						string_index++;

					string_values[string_index] += line[i];
				}
				m_Uvs.emplace_back(std::stof(string_values[0]),
					std::stof(string_values[1]));
			}
			else if (line[0] == 'v' && line[1] == 'n') {
				float values[3];
				int32_t token_start_index = 3;
				int32_t axis_index = 0;
				for (int32_t i = 3; i < line.length() + 1; i++)
				{
					if (line[i] == ' ' || line[i] == 0)
					{
						values[axis_index] = std::stof(line.substr(token_start_index, i - token_start_index));

						token_start_index = i + 1;
						axis_index++;
					}
				}
				m_Normals.emplace_back(values[0], values[1], values[2]);
			}
			else if (line[0] == 'f' && line[1] == ' ') {
				m_ObjDescs.back().primitive_count++;

				int32_t token_start_index = 2;
				int32_t dest_array_index = 0;
				for (int32_t i = 3; i < line.size() + 1; i++)
				{
					if (line[i] == '/' || line[i] == ' ' || line[i] == 0)
					{
						int32_t value;
						if (i - token_start_index == 0)
							value = -1;
						else
							value = std::stoi(line.substr(token_start_index, i - token_start_index));

						// OBJ uses 1-based array, converting 0-based arrays
						value--;

						if (dest_array_index == 0)
							m_IndexPos.push_back(value);
						else if (dest_array_index == 1)
							m_IndexUV.push_back(value);
						else
							m_IndexNrm.push_back(value);

						dest_array_index = (dest_array_index + 1) % 3;
						token_start_index = i + 1;
					}
				}
			}
			else if (line[0] == 'o' && line[1] == ' ')
			{
				mesh_names.emplace_back(line.substr(2));

				m_ObjDescs.push_back({(uint32_t)m_IndexPos.size(), 0 });
			}
		}

		std::cout << "Loaded obj scene in memory: " << timer.ElapsedMillis() << "ms\n";

		CreateInterleavedVertexBuffer();

		for (int32_t i = 0; i < m_ObjDescs.size(); i++)
		{
			auto& mesh_name = mesh_names[i];

			uint32_t primitive_count = m_ObjDescs[i].primitive_count;

			std::vector<Triangle> triangle_data;
			triangle_data.reserve(primitive_count);

			for (int32_t j = 0; j < primitive_count; j++)
			{
				uint32_t offset = m_ObjDescs[i].index_offset + j * 3;

				uint32_t* pPosIndices = &m_IndexPos[offset];
				uint32_t* pNrmIndices = &m_IndexNrm[offset];
				uint32_t* pUvsIndices = &m_IndexUV[offset];

				triangle_data.emplace_back(m_Vertices[pPosIndices[0]],
					m_Vertices[pPosIndices[1]],
					m_Vertices[pPosIndices[2]],
					m_Uvs[pUvsIndices[0]],
					m_Uvs[pUvsIndices[1]],
					m_Uvs[pUvsIndices[2]],
					m_Normals[pNrmIndices[0]],
					m_Normals[pNrmIndices[1]],
					m_Normals[pNrmIndices[2]]);
			}

			m_Materials.emplace_back(new Metal(Vector3(0.6f, 0.45f, 0.1f), 0.25f));
			m_Materials.emplace_back(new Lambertian(Vector3(0.8f)));
			m_Hittables.Add(std::make_shared<Mesh>(triangle_data, m_Materials.back(), mesh_name));
			m_Meshes.push_back(new Mesh(triangle_data, m_Materials.back(), mesh_name));
		}

		std::printf("Model loaded!\n");
	}

	void Scene::CreateInterleavedVertexBuffer()
	{
		std::unordered_set<uint32_t> set;
		for (int32_t i = 0; i < m_IndexUV.size(); i++)
		{
			uint32_t uv_index = m_IndexUV[i];
			uint32_t pos_index = m_IndexPos[i];
			uint32_t nrm_index = m_IndexNrm[i];

			if (set.find(uv_index) != set.end())
				continue;

			set.insert(uv_index);

			Vector3 pos = m_Vertices[pos_index];
			Vector3 nrm = m_Normals[nrm_index];
			Vector2 uvs = m_Uvs[uv_index];

			m_VertexBuffer.push_back({ pos, nrm, uvs });
		}

		std::cout << "Interleaved vertex buffer created!\n";
	}

	void Scene::LoadEnvMap(const std::string& path)
	{
		m_EnvMap = new ImageTexture(path, 4);
	}
};
