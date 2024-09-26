#pragma once

#include "Ray.h"
#include "Texture.h"
#include "Math/Random.h"

namespace LightBox
{
	class Material
	{
	public:
		virtual ~Material() = default;

		virtual bool Scatter(const Ray& ray, const HitRecord& result, 
			Vector3& attenuation, Ray& scattered) const = 0;

		virtual Vector3 Emitted(float u, float v, const Vector3& p) const { return Vector3(0.f); }
	private:
	};

	class DiffuseLight : public Material
	{
	public:
		DiffuseLight(Texture* tex)
			: m_Texture(tex) {}
		DiffuseLight(const Vector3& emit)
			: m_Texture(new SolidColor(emit)) {}

		Vector3 Emitted(float u, float v, const Vector3& p) const override
		{
			return m_Texture->GetValue(u, v, p);
		}

		bool Scatter(const Ray& ray, const HitRecord& result, Vector3& attenuation, Ray& scattered) const override
		{
			return false;
		}

	private:
		Texture* m_Texture;
	};

	class Lambertian : public Material
	{
	public:
		Lambertian(const Vector3 a)
			: m_AlbedoTex(new SolidColor(a)) {}
		Lambertian(Texture* texture)
			: m_AlbedoTex(texture) {}

		bool Scatter(const Ray& ray, const HitRecord& rec,
			Vector3& attenuation, Ray& scattered) const override
		{
			// Uniform distribution
			//Vector3 direction = Vector3::GetRandomVector3OnHemisphere(record.normal);
			// Lambertian distribution
			Vector3 scatter_direction = rec.normal + Random::GetRandomUnitVector3();
			// Catch degenerate scatter direction
			if (scatter_direction.IsNearZero())
				scatter_direction = rec.normal;

			//scattered = Ray(rec.point, scatter_direction);
			scattered.m_Origin = rec.point;
			scattered.m_Direction = scatter_direction;
			//attenuation = m_Albedo;
			attenuation = m_AlbedoTex->GetValue(rec.u, rec.v, rec.point);

			return true;
		}
	private:
		//Vector3 m_Albedo;
		Texture* m_AlbedoTex;
	};
	class Metal : public Material
	{
	public:
		Metal(const Vector3 a, float fuzz)
			: m_Albedo(a), m_Fuzz(fuzz) {}

		bool Scatter(const Ray& ray, const HitRecord& rec,
			Vector3& attenuation, Ray& scattered) const override
		{
			Vector3 reflected = Vector3::Reflect(ray.GetDirection().Normalize(), rec.normal)
				+ m_Fuzz * Random::GetRandomUnitVector3();

			//scattered = Ray(rec.point, reflected);
			scattered.m_Origin = rec.point;
			scattered.m_Direction = reflected;
			attenuation = m_Albedo;
			return (Vector3::Dot(reflected, rec.normal) > 0);
		}
	public:
		Vector3 m_Albedo;
		float m_Fuzz;
	};
	class Dielectric : public Material
	{
	public:
		Dielectric(float ir)
			: m_IndexOfRefraction(ir) {}

		bool Scatter(const Ray& ray, const HitRecord& rec,
			Vector3& attenuation, Ray& scattered) const override
		{
			attenuation = Vector3(1.f, 1.f, 1.f);
			float refraction_ratio = rec.front_face ? (1.f / m_IndexOfRefraction) : m_IndexOfRefraction;
			Vector3 unit_in_direction = ray.m_Direction.Normalize();
			float cos_theta = fmin(Vector3::Dot(-unit_in_direction, rec.normal), 1.f);
			float sin_theta = sqrt(1.f - cos_theta * cos_theta);

			bool cannot_refract = refraction_ratio * sin_theta > 1.f;

			Vector3 direction;
			if (cannot_refract || Reflectance(cos_theta, refraction_ratio) > Random::Float(0.f, 1.f))
				direction = Vector3::Reflect(unit_in_direction, rec.normal);
			else
				direction = Vector3::Refract(unit_in_direction, rec.normal, refraction_ratio, cos_theta);

			//scattered = Ray(rec.point, direction);
			scattered.m_Origin = rec.point;
			scattered.m_Direction = direction;
			return true;
		}
	private:
		static float Reflectance(float cosine, float refraction_ratio) {
			// Use Schlick's approximation for reflectance.
			float r0 = (1 - refraction_ratio) / (1 + refraction_ratio);
			r0 = r0 * r0;
			return r0 + (1 - r0) * (float)pow((1 - cosine), 5);
		}
	public:
		float m_IndexOfRefraction;
	};
}
