#pragma once

#pragma once

#include <cmath>
#include <iostream>

namespace LightBox {
	class Vector3;
	class Vector4 {
	public:
		Vector4() = default;
		Vector4(float x, float y, float z, float w)
			: x(x), y(y), z(z), w(w) {}
		Vector4(float value)
			: x(value), y(value), z(value), w(value) {}
		Vector4(const Vector3& v, float w);

		float x, y, z, w;

		float GetLength() const {
			return sqrt(GetLengthSq());
		}
		float GetLengthSq() const {
			return x * x + y * y + z * z + w * w;
		}
	//	Vector3 Normalize() const {
	//		float length = GetLength();
	//		return Vector3(x / length, y / length, z / length);
	//	}
	//	static float Dot(const Vector3& u, const Vector3& v) {
	//		return u.x * v.x + u.y * v.y + u.z * v.z;
	//	}
	//	static Vector3 Cross(const Vector3& u, const Vector3& v) {
	//		return Vector3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
	//	}
	//	static Vector3 GetRandomVector3() {
	//		return Vector3(Random::GetRandomFloat(), Random::GetRandomFloat(), Random::GetRandomFloat());
	//	}
	//	static Vector3 GetRandomVector3(double min, double max) {
	//		return Vector3(Random::GetRandomFloat(min, max), Random::GetRandomFloat(min, max), Random::GetRandomFloat(min, max));
	//	}
	//	static Vector3 random(double min, double max) {
	//		return Vector3(Random::random_double(min, max), Random::random_double(min, max), Random::random_double(min, max));
	//	}
	//	static Vector3 random_in_unit_sphere() {
	//		while (true) {
	//			Vector3 p = random(-1, 1);
	//			if (p.GetLengthSq() < 1)
	//				return p;
	//		}
	//	}
	//	static Vector3 GetRandomUnitVector3() {
	//		// Ray Tracing in One Weekend C++ implementation
	//		return GetRandomVector3(-1, 1).Normalize();
	//		// Ray Tracing in One Weekend C implementation
	//		return random_in_unit_sphere().Normalize();
	//	}
	//	static Vector3 GetRandomVector3OnHemisphere(const Vector3& normal) {
	//		Vector3 unit_vector = GetRandomUnitVector3();
	//		if (Vector3::Dot(normal, unit_vector) > 0.0f)
	//			return unit_vector;
	//		else
	//			return -unit_vector;
	//	}
	//	Vector3 operator-() const { return Vector3(-x, -y, -z); }

	//	friend std::ostream& operator<<(std::ostream& os, const Vector3& vec);
	};
	//inline Vector3 operator+(const Vector3& u, const Vector3& v) {
	//	return Vector3(u.x + v.x, u.y + v.y, u.z + v.z);
	//}
	//inline Vector3 operator-(const Vector3& u, const Vector3& v) {
	//	return Vector3(u.x - v.x, u.y - v.y, u.z - v.z);
	//}
	//inline Vector3* operator+=(Vector3& u, const Vector3& v) {

	//}
	//inline Vector3 operator*(float t, const Vector3& u) {
	//	return Vector3(t * u.x, t * u.y, t * u.z);
	//}
	//inline Vector3 operator*(const Vector3& u, float t) {
	//	return Vector3(t * u.x, t * u.y, t * u.z);
	//}
	//inline Vector3 operator/(const Vector3& u, float t) {
	//	return Vector3(u.x / t, u.y / t, u.z / t);
	//}
}

