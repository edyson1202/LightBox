#pragma once

#include "Vector3.h"
#include "Vector4.h"

namespace LightBox {
	class Mat4 final {
	public:
		Mat4();

		Mat4 GetInverse();
		void PrintMatrix();

		static Mat4 GetProjectionMatrix(float fov, float aspect, float near, float far);
		static Mat4 GetScaleMatrix(const Vector3& vec);
		static Mat4 GetTranslationMatrix(const Vector3& vec);
		static Mat4 GetYRotationMatrix(float angle);
		static Mat4 GetZRotationMatrix(float angle);
		static Mat4 Multiply(const Mat4& a, const Mat4& b);

		
	public:
		float m[4][4] = { 0 };
	};
	inline Vector4 operator*(const Mat4& mat, const Vector4& v) {
		Vector4 result;
		//result.x = mat.m[0][0] * v.x + mat.m[0][1] * v.y + mat.m[0][2] * v.z + mat.m[0][3] * v.w;
		//result.y = mat.m[1][0] * v.x + mat.m[1][1] * v.y + mat.m[1][2] * v.z + mat.m[1][3] * v.w;
		//result.z = mat.m[2][0] * v.x + mat.m[2][1] * v.y + mat.m[2][2] * v.z + mat.m[2][3] * v.w;
		//result.w = mat.m[3][0] * v.x + mat.m[3][1] * v.y + mat.m[3][2] * v.z + mat.m[3][3] * v.w;

		result.x = mat.m[0][0] * v.x + mat.m[1][0] * v.y + mat.m[2][0] * v.z + mat.m[3][0] * v.w;
		result.y = mat.m[0][1] * v.x + mat.m[1][1] * v.y + mat.m[2][1] * v.z + mat.m[3][1] * v.w;
		result.z = mat.m[0][2] * v.x + mat.m[1][2] * v.y + mat.m[2][2] * v.z + mat.m[3][2] * v.w;
		result.w = mat.m[0][3] * v.x + mat.m[1][3] * v.y + mat.m[2][3] * v.z + mat.m[3][3] * v.w;

		return result;
	}
}
