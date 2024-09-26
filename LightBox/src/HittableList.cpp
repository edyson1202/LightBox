#include "HittableList.h"

namespace LightBox
{
	HittableList::HittableList(const std::shared_ptr<Hittable>& object)
	{
		Add(object);
	}
	void HittableList::Clear()
	{
		m_Objects.clear();
	}
	void HittableList::Add(const std::shared_ptr<Hittable>& object)
	{
		m_Objects.push_back(object);
		m_BoundingBox = AABB(object->BoundingBox());
	}
	bool HittableList::Hit(const Ray& ray, Interval ray_t, HitRecord& rec) const
	{
		bool hit_anything = false;
		float closest_so_far = ray_t.max;

		for (const std::shared_ptr<Hittable>& object : m_Objects) {
			if (object->Hit(ray, Interval(ray_t.min, closest_so_far), rec)) {
				hit_anything = true;
				closest_so_far = rec.t;
			}
		}

		return hit_anything;
	}
}