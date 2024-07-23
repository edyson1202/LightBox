#pragma once

#include <vector>
#include <string>

#include "Triangle.h"
#include "Hittable.h"
#include "Vector3.h"
#include "AABB.h"
#include "Interval.h"
#include "Material.h"
#include "Node.h"


namespace LightBox {
	class Mesh : public Hittable
	{
	public:
		Mesh(std::vector<Triangle>& mesh_data, Material* material)
		{
			tris = mesh_data;

			m_BVH_Root = Node(tris, 0, mesh_data.size());
			std::cout << m_BVH_Root.m_Min << std::endl;
			std::cout << m_BVH_Root.m_Max << std::endl;

			m_BVH_Root.Split(5);

			m_Mat = material;
		}

		bool Hit(const Ray& ray, Interval ray_t, HitRecord& rec) const override;

		AABB BoundingBox() const override { return m_BoundingBox; }

		static std::vector<Triangle> GetMeshDataFromOBJ(std::string path);

	private:

	private:
		std::vector<Triangle> tris;
		Node m_BVH_Root;

		AABB m_BoundingBox;

		Material* m_Mat;
	};
}
