#pragma once

#include "Hittable.h"
#include "Vector3.h"
#include "AABB.h"
#include "Interval.h"
#include "Material.h"

namespace LightBox {
	class Triangle : public Hittable
	{
	public:
		Triangle(Vector3& vert0, Vector3& vert1, Vector3& vert2) {
			vertices[0] = vert0;
			vertices[1] = vert1;
			vertices[2] = vert2;

			float min_x = std::min(std::min(vert0.x, vert1.x), std::min(vert1.x, vert2.x));
			float min_y = std::min(std::min(vert0.y, vert1.y), std::min(vert1.y, vert2.y));
			float min_z = std::min(std::min(vert0.z, vert1.z), std::min(vert1.z, vert2.z));

			float max_x = std::max(std::max(vert0.x, vert1.x), std::max(vert1.x, vert2.x));
			float max_y = std::max(std::max(vert0.y, vert1.y), std::max(vert1.y, vert2.y));
			float max_z = std::max(std::max(vert0.z, vert1.z), std::max(vert1.z, vert2.z));

			m_BoundingBox = AABB(Interval(min_x, max_x), Interval(min_y, max_y), Interval(min_z, max_z));

			// Calculate triangle's normal
			Vector3 v0 = vertices[1] - vertices[0];
			Vector3 v1 = vertices[2] - vertices[0];
			m_Normal = Vector3::Cross(v0, v1).Normalize();

			D = -Vector3::Dot(m_Normal, vertices[0]);
		}

		bool Hit(const Ray& ray, Interval ray_t, HitRecord& rec) const override;

		AABB BoundingBox() const override { return m_BoundingBox; }
	public:
		Vector3 vertices[3];
	private:
		AABB m_BoundingBox;
		Vector3 m_Normal;
		float D;	// distance from the world origin to the tringle's plane
	};
}
