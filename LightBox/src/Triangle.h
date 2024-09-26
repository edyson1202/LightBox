#pragma once

#include "Hittable.h"
#include "Math/Vector3.h"
#include "AABB.h"
#include "Interval.h"
#include "Material.h"
#include "Math/Vector2.h"

namespace LightBox {
	// TODO change members order to reduce padding and decrease memory requirements
	class Triangle : public Hittable
	{
	public:
		Triangle() = default;
		Triangle(const Vector3& vert0, const Vector3& vert1, const Vector3& vert2,
			const Vector2& vert_uv_0, const Vector2& vert_uv_1, const Vector2& vert_uv_2, 
			const Vector3& norm0, const Vector3& norm1, const Vector3& norm2)
		{
			vertices[0] = vert0;
			vertices[1] = vert1;
			vertices[2] = vert2;

			m_Normals[0] = norm0;
			m_Normals[1] = norm1;
			m_Normals[2] = norm2;

			uvs[0] = vert_uv_0;
			uvs[1] = vert_uv_1;
			uvs[2] = vert_uv_2;

			// Precompute triangle's normal
			Vector3 v0 = vertices[1] - vertices[0];
			Vector3 v1 = vertices[2] - vertices[0];
			m_Normal = Vector3::Cross(v0, v1).Normalize();

			// Precompute distance between origin and triangle plane
			D = -Vector3::Dot(m_Normal, vertices[0]);

			// Precompute triangle min and max for faster BVH building
			/*m_Min = Vector3::Min(Vector3::Min(vertices[0], vertices[1]), vertices[2]);
			m_Max = Vector3::Max(Vector3::Max(vertices[0], vertices[1]), vertices[2]);*/
		}

		bool Hit(const Ray& ray, Interval ray_t, HitRecord& rec) const override;
		void GetTriangleUv(const Vector3& point, float& u, float& v) const;

		AABB BoundingBox() const override { return m_BoundingBox; }

		Vector3 GetCenter() const;
	public:
		Vector3 vertices[3];
		Vector3 m_Normals[3];

		Vector2 uvs[3];

		Vector3 m_Min;
		Vector3 m_Max;
	private:
		AABB m_BoundingBox;
		Vector3 m_Normal;
		float D;	// distance from the world origin to the triangle's plane
	};
}
