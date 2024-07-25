#include "Quat.h"

namespace LightBox {
	Quat::Quat(float w, float x, float y, float z)
		:w(w), x(x), y(y), z(z) {}
	Quat::Quat(float w, const Vector3& v)
		: w(w), x(v.x), y(v.y), z(v.z) {}
	float Quat::GetLength()
	{
		return sqrt(w * w + x * x + y * y + z * z);
	}
	Quat Quat::Normalize()
	{
		float length = GetLength();
		return Quat(w / length, x / length, y / length, z / length);
	}
	Vector3 Quat::Rotate(const Quat& q, const Vector3& v)
	{
		Quat q_inverse(q.w, -q.x, -q.y, -q.z);
		Quat p(0, v);

		Quat result = HamiltonProduct(HamiltonProduct(q, p), q_inverse);

		return Vector3(result.x, result.y, result.z);
	}
	Quat Quat::AngleAxis(float angle, const Vector3& v)
	{
		float half_angle = angle * 0.5f;
		float real = cos(half_angle);
		Vector3 imaginary = sin(half_angle) * v;

		return Quat(real, imaginary);
	}
	Quat Quat::HamiltonProduct(const Quat& q1, const Quat& q2)
	{
		float w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
		float x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
		float y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
		float z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;
		return Quat(w, x, y, z);
	}
}