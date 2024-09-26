#include "Sphere.h"

#include <iostream>

namespace LightBox {
	bool Sphere::Hit(const Ray& ray, Interval ray_t, HitRecord& rec) const
	{
		Vector3 ac = ray.GetOrigin() - m_Center;
		float a = Vector3::Dot(ray.GetDirection(), ray.GetDirection());
		//float b = 2.0f * Vector3::Dot(ray.GetDirection(), ac);
		float half_b = Vector3::Dot(ray.GetDirection(), ac);
		float c = Vector3::Dot(ac, ac) - m_Radius * m_Radius;

		//float discriminant = b * b - 4 * a * c;
		float discriminant = half_b * half_b - a * c;

		if (discriminant < 0)
			return false;
		//float root = (-b - sqrt(discriminant)) / (2.0f * a);
		float root = (-half_b - sqrt(discriminant)) / (a);
		if (!ray_t.Surrounds(root)) {
			//root = (-b + sqrt(discriminant)) / (2.0f * a);
			root = (-half_b + sqrt(discriminant)) / (a);
			if (!ray_t.Surrounds(root))
				return false;
		}

		rec.t = root;
		rec.point = ray.GetRayAt(rec.t);
		Vector3 outward_normal = (rec.point - m_Center) / m_Radius;
		rec.normal = (rec.point - m_Center) / m_Radius;
		rec.SetFaceNormal(ray, outward_normal);
		rec.mat = m_Mat;
		GetSphereUV(outward_normal, rec.u, rec.v);

		return true;
	}

	void Sphere::GetSphereUV(const Vector3& p, float& u, float& v)
	{
		float theta = std::acos(-p.y);
		float phi = std::atan2(-p.z, p.x) + pi;

		u = phi / (2 * pi);
		v = theta / pi;
	}
}
