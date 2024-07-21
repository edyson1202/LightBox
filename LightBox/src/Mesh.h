#pragma once

#include <vector>
#include <string>

#include "Triangle.h"
#include "Hittable.h"
#include "Vector3.h"
#include "AABB.h"
#include "Interval.h"
#include "Material.h"



namespace LightBox {
	class Mesh : public Hittable
	{
	public:
		Mesh(std::vector<Triangle>& mesh_data, Material* material) {

			tris = mesh_data;

			float min_x = 0;
			float min_y = 0;
			float min_z = 0;
			float max_x = 0;
			float max_y = 0;
			float max_z = 0;

			for (int32_t i = 0; i < tris.size(); i++) {
				Vector3& vert0 = tris[i].vertices[0];
				Vector3& vert1 = tris[i].vertices[1];
				Vector3& vert2 = tris[i].vertices[2];

				float local_min_x = std::min(std::min(vert0.x, vert1.x), std::min(vert1.x, vert2.x));
				float local_min_y = std::min(std::min(vert0.y, vert1.y), std::min(vert1.y, vert2.y));
				float local_min_z = std::min(std::min(vert0.z, vert1.z), std::min(vert1.z, vert2.z));
					
				float local_max_x = std::max(std::max(vert0.x, vert1.x), std::max(vert1.x, vert2.x));
				float local_max_y = std::max(std::max(vert0.y, vert1.y), std::max(vert1.y, vert2.y));
				float local_max_z = std::max(std::max(vert0.z, vert1.z), std::max(vert1.z, vert2.z));

				min_x = std::min(local_min_x, min_x);
				min_y = std::min(local_min_y, min_y);
				min_z = std::min(local_min_z, min_z);
				max_x = std::max(local_max_x, max_x);
				max_y = std::max(local_max_y, max_y);
				max_z = std::max(local_max_z, max_z);
			}

			m_BoundingBox = AABB(Interval(min_x, max_x), Interval(min_y, max_y), Interval(min_z, max_z));
			m_Mat = material;
		}

		bool Hit(const Ray& ray, Interval ray_t, HitRecord& rec) const override;

		AABB BoundingBox() const override { return m_BoundingBox; }

		static std::vector<Triangle> GetMeshDataFromOBJ(std::string path);

	private:
		std::vector<Triangle> tris;
		AABB m_BoundingBox;

		Material* m_Mat;
	};
}
