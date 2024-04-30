#pragma once

#include <cstdint>

#include "Ray.h"
#include "Vector3.h"
#include "HittableList.h"

namespace LightBox {
	// Ray Tracing In One Weekend Camera
	class RTIOWCamera
	{
	public:
		RTIOWCamera() = default;
		RTIOWCamera(Vector3 camera_center, float focal_length, uint32_t image_width, uint32_t image_height);
		uint32_t* Render();
	private:
		uint32_t m_ImageWidth = 800, m_ImageHeight;
		float m_AspectRatio = 16.0f / 9.0f;

		Vector3 m_CameraCenter, m_ViewportUpperLeft, m_Pixel00Location;

		float m_ViewportWidth, m_ViewportHeight = 2.0f, m_FocalLength;
		float m_PixelDeltaU, m_PixelDeltaV;

		HittableList m_World;
	};
}