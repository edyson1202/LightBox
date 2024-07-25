#include "Scene.h"

#include <string>

namespace LightBox
{
	std::vector<Triangle> Scene::LoadDataFromOBJ(const std::string& path)
	{
		// TODO data loading can be optimized by loading vertices first then indices (ex. there is no need to check
		// if line starts with 'v' if all vertices were read already

		std::string str;
		std::ifstream file(path);

		while (std::getline(file, str)) {

			if (str[0] == 'v' && str[1] == ' ') {
				std::string string_values[3];
				int string_index = 0;

				for (int32_t i = 2; i < str.length(); i++) {
					if (str[i] == ' ')
						string_index++;

					string_values[string_index] += str[i];
				}
				m_Vertices.emplace_back(std::stof(string_values[0]), std::stof(string_values[1]), std::stof(string_values[2]));
			}
			if (str[0] == 'f' && str[1] == ' ') {
				std::string string_values[3];
				int string_index = 0;

				for (int32_t i = 2; i < str.length(); i++) {
					if (str[i] == ' ')
						string_index++;

					string_values[string_index] += str[i];
				}
				for (int32_t i = 0; i < 3; i++) {
					int32_t char_index = 0;
					while (string_values[i][char_index] != '/')
						char_index++;
					string_values[i].resize(char_index);
				}
				m_Indices.push_back(std::stoi(string_values[0]) - 1);
				m_Indices.push_back(std::stoi(string_values[1]) - 1);
				m_Indices.push_back(std::stoi(string_values[2]) - 1);
			}
		}

		std::vector<Triangle> mesh_data(m_Indices.size() / 3);


		for (int32_t i = 0; i < mesh_data.size(); i++)
		{
			mesh_data[i] = Triangle(m_Vertices[m_Indices[i * 3]],
				m_Vertices[m_Indices[i * 3 + 1]],
				m_Vertices[m_Indices[i * 3 + 2]]);
		}

		return mesh_data;
	}
};
