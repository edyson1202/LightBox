#pragma once

#include "Ray.h"
#include "Vector3.h"
#include "Interval.h"
#include "AABB.h"

namespace LightBox
{
	struct HitRecord
	{
		Vector3 point;
		Vector3 normal;
		class Material* mat;
		float t;
		bool front_face;

		void SetFaceNormal(const Ray& ray, Vector3 outward_normal) {
			front_face = Vector3::Dot(ray.GetDirection(), outward_normal) < 0;
			normal = front_face ? outward_normal : -outward_normal;
		}
	};
	class Hittable
	{
	public:
		virtual ~Hittable() = default;

		virtual bool Hit(const Ray& ray, Interval ray_t, HitRecord& rec) const = 0;

		virtual AABB BoundingBox() const = 0;
	};
}

