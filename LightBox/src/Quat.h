#pragma once

#include "Vector3.h"

namespace LightBox {
	class Quat {
	public:
		Quat(float w, float x, float y, float z);
		Quat(float w, const Vector3& v);

		float GetLength();
		Quat Normalize();

		static Vector3 Rotate(const Quat& q, const Vector3& v);
		static Quat AngleAxis(float angle, const Vector3& v);
		static Quat HamiltonProduct(const Quat& q1, const Quat& q2);

		float w, x, y, z;
	};
}