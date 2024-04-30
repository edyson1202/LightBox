#include "Mat4.h"

#include <math.h>
#include <iostream>
#include <cstdint>

#include "Vector3.h"

// Matrices are column major
// This is how you should visualize the layout in memory
// [0][0] [1][0] [2][0] [3][0]
// [0][1] [1][1] [2][1] [3][1]
// [0][2] [1][2] [2][2] [3][2]
// [0][3] [1][3] [2][3] [3][3]
namespace LightBox {
	float ToRadians(float degrees) {
		return degrees * (3.14159f / 180.f);
	}

	Mat4::Mat4()
	{
		m[0][0] = 1;
		m[1][1] = 1;
		m[2][2] = 1;
		m[3][3] = 1;
	}
	Mat4 Mat4::GetInverse()
	{
		Mat4 temp = *this;
		Mat4 inverse;

		for (uint32_t x = 0; x < 4; x++) {
			if (temp.m[x][x] == 0) {
				std::cout << "Matrix is singular" << std::endl;
				return temp;
			}
			for (uint32_t y = 0; y < 4; y++) {
				if (x == y)
					continue;
				float ratio = -temp.m[y][x] / temp.m[x][x];
				for (uint32_t k = 0; k < 4; k++) {
					temp.m[y][k] = temp.m[y][k] + ratio * temp.m[x][k];
					inverse.m[y][k] = inverse.m[y][k] + ratio * inverse.m[x][k];
				}
			}
		}
		for (uint32_t i = 0; i < 4; i++)
			for (uint32_t j = 0; j < 4; j++)
				inverse.m[i][j] = inverse.m[i][j] / temp.m[i][i];
		for (uint32_t i = 0; i < 4; i++)
			for (uint32_t j = 0; j < 4; j++)
				temp.m[i][j] = temp.m[i][j] / temp.m[i][i];

		return inverse;
	}
	void Mat4::PrintMatrix()
	{
		for (uint32_t i = 0; i < 4; i++) {
			for (uint32_t j = 0; j < 4; j++) {
				std::cout << m[i][j] << " ";
			}
			std::cout << "\n";
		}
		std::cout << "\n\n";
	}
	/// Build a projection matrix
	/// 
	/// @param fov in degrees.
	/// @param aspect is defined as height / width.
	/// @return the corresponding projection matrix.
	Mat4 Mat4::GetProjectionMatrix(float fov, float aspect, float near, float far)
	{
		// Matrix has column-major form		
		Mat4 mat;
		float fov_in_radians = (fov * 3.14159f) / 180.f;
		float f = 1.f / tanf(fov_in_radians / 2.f);

		mat.m[0][0] = aspect * f;
		mat.m[1][1] = -f;
		mat.m[2][2] = -far / (far - near);
		mat.m[3][2] = (-far * near) / (far - near);
		mat.m[2][3] = -1.f;
		
		return mat;
	}
	Mat4 Mat4::GetScaleMatrix(const Vector3& vec) {
		Mat4 mat;
		mat.m[0][0] = vec.x;
		mat.m[1][1] = vec.y;
		mat.m[2][2] = vec.z;

		return mat;
	}
	Mat4 Mat4::GetTranslationMatrix(const Vector3& vec)
	{
		Mat4 mat;
		mat.m[0][0] = 1.f;
		mat.m[1][1] = 1.f;
		mat.m[2][2] = 1.f;
		mat.m[3][3] = 1.f;

		mat.m[3][0] = vec.x;
		mat.m[3][1] = vec.y;
		mat.m[3][2] = vec.z;

		return mat;
	}
	Mat4 Mat4::GetYRotationMatrix(float angle)
	{
		float theta = ToRadians(angle);
		Mat4 mat;
		mat.m[0][0] = cos(theta);
		mat.m[0][2] = -sin(theta);
		mat.m[2][0] = sin(theta);
		mat.m[2][2] = cos(theta);
		mat.m[1][1] = 1.f;
		mat.m[3][3] = 1.f;
		return mat;
	}
	Mat4 Mat4::GetZRotationMatrix(float angle)
	{
		float theta = ToRadians(angle);
		Mat4 mat;
		mat.m[0][0] = cos(theta);
		mat.m[1][0] = -sin(theta);
		mat.m[0][1] = sin(theta);
		mat.m[1][1] = cos(theta);
		mat.m[2][2] = 1.f;
		mat.m[3][3] = 1.f;
		return mat;
	}
	Mat4 Mat4::Multiply(const Mat4& a, const Mat4& b)
	{
		Mat4 mat;
		for (uint32_t i = 0; i < 4; i++) {
			for (uint32_t j = 0; j < 4; j++) {
				mat.m[i][j] = a.m[i][0] * b.m[0][j] +
					          a.m[i][1] * b.m[1][j] +
					          a.m[i][2] * b.m[2][j] +
					          a.m[i][3] * b.m[3][j];
			}
		}
		return mat;
	}
}