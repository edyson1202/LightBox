#include "Mesh.h"

#include <iostream>
#include <fstream>

namespace LightBox
{
	bool Mesh::Hit(const Ray& ray, Interval ray_t, HitRecord& rec) const
	{
		if (!m_BVH_Root.Hit(ray, ray_t, rec))
			return false;

		rec.mat = m_Mat;

		return true;

		if (!m_BVH_Root.m_BoundingBox.Hit(ray, ray_t))
			return false;

		bool hit_anything = false;
		float closest_so_far = ray_t.max;

		for (int32_t i = 0; i < tris.size(); i++) {
			if (tris[i].Hit(ray, Interval(ray_t.min, closest_so_far), rec)) {
				rec.mat = m_Mat;
				hit_anything = true;
				closest_so_far = rec.t;
			}
		}
		return hit_anything;
	}
	std::vector<Triangle> Mesh::GetMeshDataFromOBJ(std::string path)
	{
		std::vector<Triangle> mesh_data;
		std::vector<Vector3> vertices;
		std::vector<Vector3> indices;

		std::string str;
		std::ifstream file(path);

		while (std::getline(file, str)) {

			//std::cout << str << std::endl;

			if (str[0] == 'v' && str[1] == ' ') {
				std::string string_values[3];
				int string_index = 0;

				for (int32_t i = 2; i < str.length(); i++) {
					if (str[i] == ' ')
						string_index++;

					string_values[string_index] += str[i];
				}
				vertices.push_back(Vector3(std::stof(string_values[0]), std::stof(string_values[1]), std::stof(string_values[2])));
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
				indices.push_back(Vector3(std::stoi(string_values[0]) - 1, std::stoi(string_values[1]) - 1, std::stoi(string_values[2]) - 1));
			}
		}

		for (int32_t i = 0; i < indices.size(); i++) {
			mesh_data.push_back(Triangle(vertices[(int)indices[i].x], vertices[(int)indices[i].y], vertices[(int)indices[i].z]));
		}

		return mesh_data;
	}
}