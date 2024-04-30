#include "RTIOWCamera.h"

#include <iostream>

#include "Sphere.h"

namespace LightBox {
	RTIOWCamera::RTIOWCamera(Vector3 camera_center, float focal_length, uint32_t image_width, uint32_t image_height)
		: m_CameraCenter(camera_center), m_FocalLength(focal_length), m_ImageWidth(image_width), m_ImageHeight(image_height)
	{
		//m_ImageHeight = (uint32_t)(m_ImageWidth / m_AspectRatio);
		//m_ImageHeight = (m_ImageHeight < 1) ? 1 : m_ImageHeight;

		m_ViewportWidth = m_ViewportHeight * (float)m_ImageWidth / m_ImageHeight;

		m_PixelDeltaU = m_ViewportWidth / m_ImageWidth;
		m_PixelDeltaV = m_ViewportHeight / m_ImageHeight;

		m_ViewportUpperLeft = Vector3(-0.5f * m_ViewportWidth, 0.5f * m_ViewportHeight, -m_FocalLength);
		m_Pixel00Location = Vector3(m_ViewportUpperLeft.x + 0.5f * m_PixelDeltaU, m_ViewportUpperLeft.y - 0.5f * m_PixelDeltaV, m_ViewportUpperLeft.z);

		//m_World.Add(std::make_shared<Sphere>(Vector3(0, 0, -2.5f), 0.5f));
		//m_World.Add(std::make_shared<Sphere>(Vector3(1, 0, -2.5f), 0.5f));
		m_World.Add(std::make_shared<Sphere>(Vector3(0, 0, -1.0f), 0.5f));
		m_World.Add(std::make_shared<Sphere>(Vector3(0, -100.5f, -1), 100));
	}
	uint32_t* RTIOWCamera::Render()
	{
		uint32_t* data = new uint32_t[m_ImageWidth * m_ImageHeight];

		for (uint32_t y = 0; y < m_ImageHeight; y++)
			for (uint32_t x = 0; x < m_ImageWidth; x++) {
				Vector3 pixel_location = Vector3(m_Pixel00Location.x + x * m_PixelDeltaU, m_Pixel00Location.y - y * m_PixelDeltaV, m_Pixel00Location.z);
				Vector3 ray_direction = pixel_location - m_CameraCenter;

				Ray ray(m_CameraCenter, ray_direction);
				Vector3 color(0.0f);
				uint32_t samples = 50;
				for (uint32_t i = 0; i < samples; i++) {
					color = color + (TraceRay(ray, 20, m_World));
				}
				color = color / samples;
				color = 255.0f * color;

				data[y * m_ImageWidth + x] = 0xff000000;
				data[y * m_ImageWidth + x] |= (uint32_t)color.x;
				data[y * m_ImageWidth + x] |= ((uint32_t)color.y) << 8;
				data[y * m_ImageWidth + x] |= ((uint32_t)color.z) << 16;
			}
		return data;
	}
}