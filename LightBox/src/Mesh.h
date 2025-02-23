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
		Mesh(std::vector<Triangle>& mesh_data, Material* material, const std::string& name)
			: tris(mesh_data),
		m_Mat(material),
		m_BVH_Root(tris, 0, tris.size()),
		m_Name(name)
		{
			//m_BVH_Root = Node(tris, 0, mesh_data.size());
			m_BVH_Root.Split(15);
		}

		bool Hit(const Ray& ray, Interval ray_t, HitRecord& rec) const override;

		AABB BoundingBox() const override { return m_BoundingBox; }

		Node& GetBVHRootNode() { return m_BVH_Root; }
	public:
		std::string m_Name;

	private:
		std::vector<Triangle> tris;

		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;

		Material* m_Mat;

		Node m_BVH_Root;

		AABB m_BoundingBox;
	};
}
