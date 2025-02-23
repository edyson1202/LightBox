#pragma once

#include <algorithm>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include <cmath>
#include <iostream>

#include "Vector4.h"

namespace LightBox {
	class alignas(16) Vector3 {
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
		Vector3 Clamp(const Vector3& min, const Vector3& max) const
		{
			return Vector3(std::clamp(x, min.x, max.x),
				std::clamp(y, min.y, max.y),
				std::clamp(z, min.z, max.z));
		}
		static Vector3 Reflect(const Vector3& v, const Vector3& normal);
		static Vector3 Refract(const Vector3& v, const Vector3& n, float etai_over_etat, float cos_theta);
		static float Dot(const Vector3& u, const Vector3& v) {
			return u.x * v.x + u.y * v.y + u.z * v.z;
		}
		// glm Dot function wrapper to test against own implementation
		static float Dot2(const Vector3& u, const Vector3& v) {
			return glm::dot(*((glm::vec3*)&u), *((glm::vec3*)&v));
		}
		static Vector3 Cross(const Vector3& u, const Vector3& v) {
			return Vector3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
		}
		static Vector3 Min(const Vector3& u, const Vector3& v) {
			return Vector3(std::min(u.x, v.x), std::min(u.y, v.y), std::min(u.z, v.z));
		}
		static Vector3 Max(const Vector3& u, const Vector3& v) {
			return Vector3(std::max(u.x, v.x), std::max(u.y, v.y), std::max(u.z, v.z));
		}
		static Vector3 Mix(const Vector3& u, const Vector3& v, float t);

		const float& operator [] (unsigned i) const { return (&x)[i]; }
		float& operator [] (unsigned i) { return (&x)[i]; }
		Vector3 operator-() const { return Vector3(-x, -y, -z); }	

		Vector3& operator+=(const Vector3& other) {
			this->x += other.x;
			this->y += other.y;
			this->z += other.z;
			return *this; // return the updated object
		}

		friend std::ostream& operator<<(std::ostream& os, const Vector3& vec);
	};
	inline Vector3 operator+(const Vector3& u, const Vector3& v) {
		return Vector3(u.x + v.x, u.y + v.y, u.z + v.z);
	}
	inline Vector3 operator-(const Vector3& u, const Vector3& v) {
		return Vector3(u.x - v.x, u.y - v.y, u.z - v.z);
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


