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
		Vector3 c0 = P - vertices[0];

		if (Vector3::Dot(Vector3::Cross(edge0, c0), m_Normal) < 0)
			return false;

		// Edge1
		Vector3 edge1 = vertices[2] - vertices[1];
		Vector3 c1 = P - vertices[1];

		if (Vector3::Dot(Vector3::Cross(edge1, c1), m_Normal) < 0)
			return false;

		// Edge2
		Vector3 edge2 = vertices[0] - vertices[2];
		Vector3 c2 = P - vertices[2];

		if (Vector3::Dot(Vector3::Cross(edge2, c2), m_Normal) < 0)
			return false;

		rec.point = P;
		rec.t = t;
		//rec.mat = m_Mat;
		rec.SetFaceNormal(ray, m_Normal);

		return true;
	}

	Vector3 Triangle::GetCenter() const
	{
		return (vertices[0] + vertices[1] + vertices[2]) / 3;
	}
}
