#pragma once

#include <vector>
#include <string>

#include "Triangle.h"
#include "Hittable.h"
#include "Math/Vector3.h"
#include "AABB.h"
#include "Interval.h"
#include "Material.h"
#include "Node.h"


namespace LightBox {
	class Mesh : public Hittable
	{
	public:
		Mesh(std::vector<Triangle>& mesh_data, Material* material)
			: tris(mesh_data), m_Mat(material)
		{
			m_BVH_Root = Node(tris, 0, mesh_data.size());

			m_BVH_Root.Split(5);
		}

		bool Hit(const Ray& ray, Interval ray_t, HitRecord& rec) const override;

		AABB BoundingBox() const override { return m_BoundingBox; }

	private:
		std::vector<Triangle> tris;
		Node m_BVH_Root;

		AABB m_BoundingBox;

		Material* m_Mat;
	};
}
