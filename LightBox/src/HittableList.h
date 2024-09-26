#pragma once

#include "Hittable.h"

#include <vector>
#include <memory>

#include "AABB.H"

namespace LightBox
{
	class HittableList : public Hittable
	{
	public:
		HittableList() = default;
		HittableList(const std::shared_ptr<Hittable>& object);

		void Clear();
		void Add(const std::shared_ptr<Hittable>& object);

		bool Hit(const Ray& ray, Interval ray_t, HitRecord& rec) const override;

		AABB BoundingBox() const { return m_BoundingBox; }

		auto& GetHittables() const { return m_Objects; }

	public:
		std::vector<std::shared_ptr<Hittable>> m_Objects;
	private:
		AABB m_BoundingBox;
	};
}