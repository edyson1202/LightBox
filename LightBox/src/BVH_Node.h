#pragma once

#include "Hittable.h"
#include "HittableList.h"
#include "Math/Random.h"

namespace LightBox
{
    class BVH_Node : public Hittable {
    public:
        BVH_Node(const HittableList& list) 
            : BVH_Node(list.m_Objects, 0, list.m_Objects.size()) {}

        BVH_Node(const std::vector<std::shared_ptr<Hittable>>& src_objects, size_t start, size_t end) {
            auto objects = src_objects; // Create a modifiable array of the source scene objects

            int axis = Random::random_int(0, 2);
            auto comparator = (axis == 0) ? box_x_compare
                : (axis == 1) ? box_y_compare
                : box_z_compare;

            size_t object_span = end - start;

            if (object_span == 1) {
                left = right = objects[start];
            }
            else if (object_span == 2) {
                if (comparator(objects[start], objects[start + 1])) {
                    left = objects[start];
                    right = objects[start + 1];
                }
                else {
                    left = objects[start + 1];
                    right = objects[start];
                }
            }
            else {
                std::sort(objects.begin() + start, objects.begin() + end, comparator);

                auto mid = start + object_span / 2;
                left = std::make_shared<BVH_Node>(objects, start, mid);
                right = std::make_shared<BVH_Node>(objects, mid, end);
            }

            m_BoundingBox = AABB(left->BoundingBox(), right->BoundingBox());
        }

        bool Hit(const Ray& r, Interval ray_t, HitRecord& rec) const override {
            if (!m_BoundingBox.Hit(r, ray_t))
                return false;

            bool hit_left = left->Hit(r, ray_t, rec);
            bool hit_right = right->Hit(r, Interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);

            return hit_left || hit_right;
        }

        AABB BoundingBox() const override { return m_BoundingBox; }

    private:

        static bool box_compare(
            const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b, int axis_index
        ) {
            return a->BoundingBox().Axis(axis_index).min < b->BoundingBox().Axis(axis_index).min;
        }

        static bool box_x_compare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b) {
            return box_compare(a, b, 0);
        }

        static bool box_y_compare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b) {
            return box_compare(a, b, 1);
        }

        static bool box_z_compare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b) {
            return box_compare(a, b, 2);
        }
    private:
        std::shared_ptr<Hittable> left;
        std::shared_ptr<Hittable> right;
        AABB m_BoundingBox;
    };
}
