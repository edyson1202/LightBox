#pragma once

namespace LightBox {
	class Vector2 {
	public:
		Vector2() = default;
		Vector2(float x, float y);

		float x, y;
	};
	inline Vector2 operator+(const Vector2& u, const Vector2& v) {
		return Vector2(u.x + v.x, u.y + v.y);
	}
	inline Vector2 operator-(const Vector2& u, const Vector2& v) {
		return Vector2(u.x - v.x, u.y - v.y);
	}
	inline Vector2 operator-(const Vector2& v, float a) {
		return Vector2(v.x - a, v.y - a);
	}
	inline Vector2 operator*(const Vector2& v, float a) {
		return Vector2(v.x * a, v.y * a);
	}
	inline Vector2 operator*(float a, const Vector2& v) {
		return v * a;
	}
}
