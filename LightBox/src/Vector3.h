#pragma once

#include <cmath>
#include <iostream>

#include "Vector4.h"

namespace LightBox {
	class Vector3 {
	public:
		Vector3() = default;
		Vector3(float x, float y, float z)
			: x(x), y(y), z(z) {}
		Vector3(float value)
			: x(value), y(value), z(value) {}
		Vector3(const Vector4& v)
			: x(v.x), y(v.y), z(v.z) {}

		float x, y, z;
		
		float GetLength() const {
			return sqrt(GetLengthSq());
		}
		float GetLengthSq() const {
			return x * x + y * y + z * z;
		}
		Vector3 Normalize() const {
			float length = GetLength();
			return Vector3(x / length, y / length, z / length);
		}
		bool IsNearZero() const {
			auto s = 1e-8;
			return (fabs(x) < s) && (fabs(y) < s) && (fabs(z) < z);
		}
		static Vector3 Reflect(const Vector3& v, const Vector3& normal);
		static Vector3 Refract(const Vector3& v, const Vector3& n, float etai_over_etat, float cos_theta);
		static float Dot(const Vector3& u, const Vector3& v) {
			return u.x * v.x + u.y * v.y + u.z * v.z;
		}
		static Vector3 Cross(const Vector3& u, const Vector3& v) {
			return Vector3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
		}
		//static Vector3 GetRandomVector3() {
		//	return Vector3(Random::GetRandomFloat(), Random::GetRandomFloat(), Random::GetRandomFloat());
		//}
		//static Vector3 GetRandomVector3(double min, double max) {
		//	return Vector3(Random::GetRandomFloat(min, max), Random::GetRandomFloat(min, max), Random::GetRandomFloat(min, max));
		//}
		//static Vector3 random(double min, double max) {
		//	return Vector3(Random::random_double(min, max), Random::random_double(min, max), Random::random_double(min, max));
		//}
		//static Vector3 random_in_unit_sphere() {
		//	while (true) {
		//		Vector3 p = random(-1, 1);
		//		if (p.GetLengthSq() < 1)
		//			return p;
		//	}
		//}
		//static Vector3 Vec3(float min, float max)
		//{
		//	return Vector3(Random::Float() * (max - min) + min, Random::Float() * (max - min) + min, Random::Float() * (max - min) + min);
		//}
		//static Vector3 GetRandomUnitVector3() {
		//	// The Cherno
		//	return Vector3::Vec3(-1.0f, 1.0f);
		//	// Ray Tracing in One Weekend C++ implementation
		//	//return GetRandomVector3(-1, 1).Normalize();
		//	// Ray Tracing in One Weekend C implementation
		//	return random_in_unit_sphere().Normalize();
		//}
		//static Vector3 GetRandomVector3OnHemisphere(const Vector3& normal) {
		//	Vector3 unit_vector = GetRandomUnitVector3();
		//	if (Vector3::Dot(normal, unit_vector) > 0.0f)
		//		return unit_vector;
		//	else
		//		return -unit_vector;
		//}
		Vector3 operator-() const { return Vector3(-x, -y, -z); }	
		double operator[](uint32_t i) const { return ((float*)&x)[i]; }

		friend std::ostream& operator<<(std::ostream& os, const Vector3& vec);
	};
	inline Vector3 operator+(const Vector3& u, const Vector3& v) {
		return Vector3(u.x + v.x, u.y + v.y, u.z + v.z);
	}
	inline Vector3 operator-(const Vector3& u, const Vector3& v) {
		return Vector3(u.x - v.x, u.y - v.y, u.z - v.z);
	}
	inline Vector3* operator+=(Vector3& u, const Vector3& v) {
		
	}
	inline Vector3 operator*(const Vector3& u, const Vector3& v) {
		return Vector3(u.x * v.x, u.y * v.y, u.z * v.z);
	}
	inline Vector3 operator*(float t, const Vector3& u) {
		return Vector3(t * u.x, t * u.y, t * u.z);
	}
	inline Vector3 operator*(const Vector3& u, float t) {
		return Vector3(t * u.x, t * u.y, t * u.z);
	}
	inline Vector3 operator/(const Vector3& u, float t) {
		return Vector3(u.x / t, u.y / t, u.z / t);
	}
}


