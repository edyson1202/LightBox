#include "Math/Vector3.h"
#include "Hittable.h"
#include "Material.h"
#include "AABB.h"


namespace LightBox {
	class Sphere : public Hittable
	{
	public:
		Sphere(const Vector3& center, float radius, Material* material)
			: m_Center(center), m_Radius(radius), m_Mat(material) 
		{
			Vector3 radius_vector(m_Radius);
			m_BoundingBox = AABB(m_Center - radius_vector, m_Center + radius_vector);
		};

		Vector3 GetCenter() const { return m_Center; }
		float GetRadius() const { return m_Radius; }

		bool Hit(const Ray& ray, Interval ray_t, HitRecord& rec) const override;

		AABB BoundingBox() const override { return m_BoundingBox; }
	public:
		Vector3 m_Center;
		float m_Radius;

		Material* m_Mat;
		AABB m_BoundingBox;
	};
}