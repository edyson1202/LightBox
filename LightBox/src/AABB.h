#pragma once



#include "Ray.h"
#include "Interval.h"

namespace LightBox
{
	class AABB
	{
	public:
		Interval x, y, z;

		AABB() {}
		AABB(const Interval& ix, const Interval& iy, const Interval& iz)
			: x(ix), y(iy), z(iz) {}
		AABB(const Vector3& a, const Vector3& b)
		{
			x = Interval(fmin(a.x, b.x), fmax(a.x, b.x));
			y = Interval(fmin(a.y, b.y), fmax(a.y, b.y));
			z = Interval(fmin(a.z, b.z), fmax(a.z, b.z));
		}
        AABB(const AABB& box0, const AABB& box1) {
            x = Interval(box0.x, box1.x);
            y = Interval(box0.y, box1.y);
            z = Interval(box0.z, box1.z);
        }
        const Interval& Axis(int n) const {
            if (n == 1) return y;
            if (n == 2) return z;
            return x;
        }
        bool Hit2(const Ray& r, Interval ray_t) const {
            for (int a = 0; a < 3; a++) {
                auto t0 = fmin((Axis(a).min - r.GetOrigin()[a]) / r.GetDirection()[a],
                    (Axis(a).max - r.GetOrigin()[a]) / r.GetDirection()[a]);
                auto t1 = fmax((Axis(a).min - r.GetOrigin()[a]) / r.GetDirection()[a],
                    (Axis(a).max - r.GetOrigin()[a]) / r.GetDirection()[a]);
                ray_t.min = fmax(t0, ray_t.min);
                ray_t.max = fmin(t1, ray_t.max);
                if (ray_t.max <= ray_t.min)
                    return false;
            }
            return true;
        }
        bool Hit(const Ray& r, Interval ray_t) const {
            for (int a = 0; a < 3; a++) {
                auto invD = 1 / r.GetDirection()[a];
                auto orig = r.GetOrigin()[a];

                auto t0 = (Axis(a).min - orig) * invD;
                auto t1 = (Axis(a).max - orig) * invD;

                if (invD < 0)
                    std::swap(t0, t1);

                if (t0 > ray_t.min) ray_t.min = t0;
                if (t1 < ray_t.max) ray_t.max = t1;

                if (ray_t.max <= ray_t.min)
                    return false;
            }
            return true;
        }
	};
}
