#pragma once

#include "Hittable.h"
#include "Math/Vector3.h"
#include "AABB.h"
#include "Interval.h"
#include "Material.h"

namespace LightBox {
	// TODO change members order to reduce padding and decrease memory requirements
	class Triangle : public Hittable
	{
	public:
		Triangle() = default;
		Triangle(const Vector3& vert0, const Vector3& vert1, const Vector3& vert2)
		{
			vertices[0] = vert0;
			vertices[1] = vert1;
			vertices[2] = vert2;

			// Precompute triangle's normal
			Vector3 v0 = vertices[1] - vertices[0];
			Vector3 v1 = vertices[2] - vertices[0];
			m_Normal = Vector3::Cross(v0, v1).Normalize();

			// Precompute distance between origin and triangle plane
			D = -Vector3::Dot(m_Normal, vertices[0]);
		}

		bool Hit(const Ray& ray, Interval ray_t, HitRecord& rec) const override;

		AABB BoundingBox() const override { return m_BoundingBox; }

		Vector3 GetCenter() const;
	public:
		Vector3 vertices[3];
	private:
		AABB m_BoundingBox;
		Vector3 m_Normal;
		float D;	// distance from the world origin to the triangle's plane
	};
}
