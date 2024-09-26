#include "Triangle.h"

namespace LightBox
{
	bool Triangle::Hit(const Ray& ray, Interval ray_t, HitRecord& rec) const
	{
		// Determine if the ray hits the triangle plane

		// If ray parallel to the triangle return false
		float n_dot_r = Vector3::Dot(m_Normal, ray.GetDirection());

		if (n_dot_r == 0)
			return false;

		float numerator = Vector3::Dot(m_Normal, ray.GetOrigin()) + D;
		float t = -numerator / n_dot_r;

		// If t is not inside the ray interval return false
		if (!ray_t.Surrounds(t))
			return false;

		// Determine if P is inside the triangle using the inside-outside test
		Vector3 P = ray.GetRayAt(t);

		// Edge0
		Vector3 edge0 = vertices[1] - vertices[0];
		Vector3 edge2 = vertices[0] - vertices[2];
		Vector3 c0 = P - vertices[0];

		float paralelogram_area = Vector3::Cross(edge0, -edge2).GetLength();
		Vector3 c = Vector3::Cross(edge0, c0);
		float v = c.GetLength() / paralelogram_area;
		if (Vector3::Dot(c, m_Normal) < 0)
			return false;

		// Edge1
		Vector3 edge1 = vertices[2] - vertices[1];
		Vector3 c1 = P - vertices[1];

		if (Vector3::Dot(Vector3::Cross(edge1, c1), m_Normal) < 0)
			return false;

		// Edge2
		edge2 = vertices[0] - vertices[2];
		Vector3 c2 = P - vertices[2];

		c = Vector3::Cross(-edge2, c2);
		float u = c.GetLength() / paralelogram_area;
		if (Vector3::Dot(Vector3::Cross(edge2, c2), m_Normal) < 0)
			return false;

		rec.point = P;
		rec.t = t;
		//rec.mat = m_Mat;
		// SMOOTH SHADING
		Vector3 N = (1 - u - v) * m_Normals[0] + u * m_Normals[1] + v * m_Normals[2];
		rec.SetFaceNormal(ray, N);
		//GetTriangleUv(rec.point, rec.u, rec.v);

		// Set triangle uvs
		Vector2 ab_uv = uvs[1] - uvs[0];
		Vector2 ac_uv = uvs[2] - uvs[0];

		Vector2 uv_point = uvs[0] + u * ab_uv + v * ac_uv;

		rec.u = uv_point.x;
		rec.v = uv_point.y;

		return true;
	}

	Vector3 Triangle::GetCenter() const
	{
		return (vertices[0] + vertices[1] + vertices[2]) / 3;
	}
}
