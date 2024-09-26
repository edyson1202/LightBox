#include "Scene.h"

#include <string>

#include "Sphere.h"
#include "Vulkan/Image.h"

namespace LightBox
{
	void Scene::LoadDataFromObj(const std::string& path)
	{
		std::string line;
		std::ifstream file(path);

		std::vector<std::string> mesh_names;
		std::vector<Vector3> vertex_pos;
		std::vector<Vector3> vertex_normals;
		std::vector<Vector2> vertex_uvs;
		std::vector<std::vector<uint32_t>> pos_indices;
		std::vector<std::vector<uint32_t>> normals_indices;
		std::vector<std::vector<uint32_t>> uvs_indices;

		while (std::getline(file, line)) {
			if (line[0] == 'v' && line[1] == ' ') 
			{
				/*std::string string_values[3];
				int string_index = 0;

				for (int32_t i = 2; i < line.length(); i++) {
					if (line[i] == ' ')
						string_index++;

					string_values[string_index] += line[i];
				}
				vertex_pos.emplace_back(std::stof(string_values[0]),
					std::stof(string_values[1]),
					std::stof(string_values[2]));*/

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
				vertex_pos.emplace_back(values[0], values[1], values[2]);
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
				vertex_uvs.emplace_back(std::stof(string_values[0]),
					std::stof(string_values[1]));
			}
			else if (line[0] == 'v' && line[1] == 'n') {
				/*std::string string_values[3];
				int string_index = 0;

				for (int32_t i = 3; i < line.length(); i++) {
					if (line[i] == ' ')
						string_index++;

					string_values[string_index] += line[i];
				}
				vertex_normals.emplace_back(std::stof(string_values[0]),
					std::stof(string_values[1]),
					std::stof(string_values[2]));*/

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
				vertex_normals.emplace_back(values[0], values[1], values[2]);
			}
			else if (line[0] == 'f' && line[1] == ' ') {
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
							pos_indices.back().push_back(value);
						else if (dest_array_index == 1)
							uvs_indices.back().push_back(value);
						else
							normals_indices.back().push_back(value);
							
						dest_array_index = (dest_array_index + 1) % 3;
						token_start_index = i + 1;
					}
				}
			}
			else if (line[0] == 'o' && line[1] == ' ')
			{
				mesh_names.emplace_back(line.substr(2));
				pos_indices.emplace_back();
				normals_indices.emplace_back();
				uvs_indices.emplace_back();
			}
		}

		int32_t object_count = pos_indices.size();
		for (int32_t i = 0; i < object_count; i++)
		{
			auto& vert_indices = pos_indices[i];
			auto& vert_uv_indices = uvs_indices[i];
			auto& vert_norm_indicel = normals_indices[i];
			auto& mesh_name = mesh_names[i];

			std::vector<Triangle> triangle_data(vert_indices.size() / 3);

			for (int32_t j = 0; j < triangle_data.size(); j++)
			{
				triangle_data[j] = Triangle(vertex_pos[vert_indices[j * 3]],
					vertex_pos[vert_indices[j * 3 + 1]],
					vertex_pos[vert_indices[j * 3 + 2]],
					vertex_uvs[vert_uv_indices[j * 3]],
					vertex_uvs[vert_uv_indices[j * 3 + 1]],
					vertex_uvs[vert_uv_indices[j * 3 + 2]],
					vertex_normals[vert_norm_indicel[j * 3]],
					vertex_normals[vert_norm_indicel[j * 3 + 1]], 
					vertex_normals[vert_norm_indicel[j * 3 + 2]]);
			}

			//m_Meshes.emplace_back(std::move(triangle_data), m_DefaultMaterial);
			/*m_Textures.emplace_back(new ImageTexture("E:/CodingProjects/LightBox/LightBox/resources/TheodoraCabinetDiffuse.png"));
			m_Materials.emplace_back(new Lambertian(m_Textures.back()));*/

			if (mesh_name.substr(0, 5) == "glass")
			{
				m_Materials.emplace_back(new Dielectric(1.15f));
				m_Hittables.Add(std::make_shared<Mesh>(triangle_data, m_Materials.back(), mesh_name));
			}
			else if (mesh_name.substr(0, 4) == "tire")
			{
				m_Materials.emplace_back(new Metal(Vector3(0.2f, 0.2f, 0.2f), 0.8f));
				m_Hittables.Add(std::make_shared<Mesh>(triangle_data, m_Materials.back(), mesh_name));
			}
			/*else if (mesh_name.substr(0, 4) == "body")
			{
				m_Materials.emplace_back(new Metal(Vector3(0.6f, 0.1f, 0.1f), 0.95f));
				m_Hittables.Add(std::make_shared<Mesh>(triangle_data, m_Materials.back(), mesh_name));
			}*/
			else if (mesh_name == "Plane")
			{
				m_Materials.emplace_back(new DiffuseLight(Vector3(5)));
				m_Hittables.Add(std::make_shared<Mesh>(triangle_data, m_Materials.back(), mesh_name));
			}
			else if (mesh_name == "Plane.001")
			{
				m_Textures.emplace_back(new ImageTexture("E:/CodingProjects/LightBox/LightBox/resources/wood_floor.png"));
				m_Materials.emplace_back(new Lambertian(m_Textures.back()));
				//m_Materials.emplace_back(new Lambertian(Vector3(0.8f)));
				m_Hittables.Add(std::make_shared<Mesh>(triangle_data, m_Materials.back(), mesh_name));
			}
			else
			{
				//m_Hittables.Add(std::make_shared<Mesh>(triangle_data, m_Materials.back(), mesh_name));


				m_Materials.emplace_back(new Metal(Vector3(0.6f, 0.45f, 0.1f), 0.25f));
				m_Materials.emplace_back(new Lambertian(Vector3(0.8f)));
				m_Hittables.Add(std::make_shared<Mesh>(triangle_data, m_Materials.back(), mesh_name));
			}
			m_Meshes.push_back(new Mesh(triangle_data, m_Materials.back(), mesh_name));
		}

		m_Materials.emplace_back(new Lambertian(Vector3(0.65f, 0.65f, 0.5f)));
		Vector3 color_a(0.4f, 0.8f, 0.4f);
		Vector3 color_b(0.8f, 0.6f, 0.8f);
		m_Textures.emplace_back(new CheckerTexture(color_a, color_b, 0.2f));
		m_Materials.emplace_back(new Lambertian(m_Textures.back()));
		//m_Hittables.Add(std::make_shared<Sphere>(Vector3(0, -100.5f, -1), 100, &m_DefaultMaterial));

		m_Textures.emplace_back(new ImageTexture("E:/CodingProjects/LightBox/LightBox/resources/8k_earth_daymap.jpg"));
		m_Materials.emplace_back(new Lambertian(m_Textures.back()));
		
		//m_Hittables.Add(std::make_shared<Sphere>(Vector3(0.6, 0.6, 0), 0.45f, m_DefaultMaterial_04));
		m_Hittables.Add(std::make_shared<Sphere>(Vector3(1000.6, 0.6, 0), 0.5f, m_Materials.back()));

		std::printf("Model loaded!");
	}

	void Scene::LoadEnvMap(const std::string& path)
	{
		m_EnvMap = new ImageTexture(path, 4);
	}
};
