#include "Mesh.h"

#include <iostream>
#include <fstream>

namespace LightBox
{
	bool Mesh::Hit(const Ray& ray, Interval ray_t, HitRecord& rec) const
	{
		if (!m_BVH_Root.Hit(ray, ray_t, rec))
			return false;

		rec.mat = m_Mat;

		return true;

		if (!m_BVH_Root.m_BoundingBox.Hit(ray, ray_t))
			return false;

		bool hit_anything = false;
		float closest_so_far = ray_t.max;

		for (int32_t i = 0; i < tris.size(); i++) {
			if (tris[i].Hit(ray, Interval(ray_t.min, closest_so_far), rec)) {
				rec.mat = m_Mat;
				hit_anything = true;
				closest_so_far = rec.t;
			}
		}
		return hit_anything;
	}
}