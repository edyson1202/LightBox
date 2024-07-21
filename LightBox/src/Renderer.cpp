#include "Renderer.h"

#include <stb_image.h>

#include <iostream>
#include <execution>

#include "Sphere.h"
#include "Random.h"

namespace LightBox
{
	inline float LinearToGamma(float linear_component)
	{
		return sqrt(linear_component);
	}
	Vector3 pixel_sample_square() {
		// Returns a random point in the square surrounding a pixel at the origin.
		float px = -0.5f + Random::Float();
		float py = -0.5f + Random::Float();
		//return Vector3(px * pixel_delta_u) + (py * pixel_delta_v);
		return Vector3(px, py, 0.f);
	}
	Renderer::Renderer(Device& device, Camera& camera, HittableList& scene)
		: m_Device(device), m_Camera(camera), m_World(scene)
	{

		//m_HDRData = stbi_loadf("resources/quarry_cloudy_2k.hdr", &m_Width, &m_Height, &m_Channels, 0);
		m_HDRData = stbi_load("resources/hdri_02.png", &m_Width, &m_Height, &m_Channels, STBI_rgb_alpha);

	}
	void Renderer::OnResize(uint32_t width, uint32_t height)
	{
		if (m_FinalImage) {
			// no resize necessary
			if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
				return;

			m_FinalImage->Resize(width, height);
		}
		else {
			m_FinalImage = std::make_shared<Image>(m_Device, width, height, VK_FORMAT_R8G8B8A8_UNORM);
		}

		delete[] m_ImageData;
		m_ImageData = new uint32_t[width * height];
		delete[] m_AccumulationData;
		m_AccumulationData = new Vector3[width * height];

		m_ImageHorizontalIter.resize(width);
		m_ImageVerticalIter.resize(height);
		for (uint32_t i = 0; i < width; i++)
			m_ImageHorizontalIter[i] = i;
		for (uint32_t i = 0; i < height; i++)
			m_ImageVerticalIter[i] = i;
	}
	void Renderer::Render() 
	{
		Ray ray;
		ray.m_Origin = m_Camera.GetPosition();

		uint32_t m_ImageWidth = m_FinalImage->GetWidth();
		if (m_FrameIndex == 1)
			memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(Vector3));

#define MT 1
#if MT
		std::for_each(std::execution::par, m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(),
			[this](uint32_t y)
			{
				std::for_each(std::execution::par, m_ImageHorizontalIter.begin(), m_ImageHorizontalIter.end(),
				[this, y](uint32_t x)
					{
						//RayGen(x, y);
						uint32_t m_ImageWidth = m_FinalImage->GetWidth();
						Vector3 accumulated_color = PerPixel(x, y);
						
						m_ImageData[y * m_ImageWidth + x] = 0xff000000;
						m_ImageData[y * m_ImageWidth + x] |= (uint32_t)accumulated_color.x;
						m_ImageData[y * m_ImageWidth + x] |= ((uint32_t)accumulated_color.y) << 8;
						m_ImageData[y * m_ImageWidth + x] |= ((uint32_t)accumulated_color.z) << 16;
					});
			});
#else
		for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
			for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
				//RayGen(x, y);

				ray.m_Direction = m_Camera.GetRayDirections()[x + y * m_FinalImage->GetWidth()];
				//ray.m_Direction = ray.m_Direction + 0.0008f * pixel_sample_square();
				Vector3 color = 255.0f * TraceRay(ray, m_Settings.MaxDepth, m_World);


				m_AccumulationData[y * m_ImageWidth + x] = m_AccumulationData[y * m_ImageWidth + x] + color;

				Vector3 accumulated_color = m_AccumulationData[y * m_ImageWidth + x] / 
					(float)m_FrameIndex;

				m_ImageData[y * m_ImageWidth + x] = 0xff000000;
				m_ImageData[y * m_ImageWidth + x] |= (uint32_t)accumulated_color.x;
				m_ImageData[y * m_ImageWidth + x] |= ((uint32_t)accumulated_color.y) << 8;
				m_ImageData[y * m_ImageWidth + x] |= ((uint32_t)accumulated_color.z) << 16;
			}
		}
#endif
		m_FinalImage->SetData(m_ImageData);

		if (m_Settings.Accumulate)
			m_FrameIndex++;
		else
			m_FrameIndex = 1;
	}
	Vector3 Renderer::PerPixel(uint32_t x, uint32_t y)
	{
		uint32_t m_ImageWidth = m_FinalImage->GetWidth();
		Ray ray;
		ray.m_Origin = m_Camera.GetPosition();
		ray.m_Direction = m_Camera.GetRayDirections()[x + y * m_FinalImage->GetWidth()];
		//ray.m_Direction = ray.m_Direction + 0.0008f * pixel_sample_square();
		Vector3 color = 255.0f * TraceRay(ray, m_Settings.MaxDepth, m_World);

		m_AccumulationData[y * m_ImageWidth + x] = m_AccumulationData[y * m_ImageWidth + x] + color;

		return m_AccumulationData[y * m_ImageWidth + x] /
			(float)m_FrameIndex;
	}
	Vector3 Renderer::RayGen(uint32_t x, uint32_t y)
	{
		Ray ray;
		ray.m_Origin = m_Camera.GetPosition();
		ray.m_Direction = m_Camera.GetRayDirections()[x + y * m_FinalImage->GetWidth()];

		return Vector3(1.f);

		if (1) {
			if (useEnvMap) {
				float u = 0.5f + atan2(ray.GetDirection().z, ray.GetDirection().x) / (2 * (float)pi);
				float v = 0.5f + asin(-ray.GetDirection().y) / (float)pi;

				uint32_t x = u * (m_Width - 1);
				uint32_t y = v * (m_Height - 1);

				Vector3 color(0.f);
				int pixel = (y * m_Width + x) * 4;
				color.x = m_HDRData[pixel] / 255.f;
				color.y = m_HDRData[pixel + 1] / 255.f;
				color.z = m_HDRData[pixel + 2] / 255.f;

				return color;
			}
			// Sky box shading
			Vector3 dir = ray.GetDirection();
			dir = dir.Normalize();
			float a = 0.5f * (dir.y + 1.0f);

			return (1.0f - a) * Vector3(1.0f, 1.0f, 1.0f) + a * Vector3(127.5f / 255.0f, 178.5f / 255.0f, 1.0f);
		}
	}
	Renderer::HitPayload Renderer::Miss(const Ray& ray)
	{
		Renderer::HitPayload payload;
		payload.hit_distance = -1.f;

		return payload;
	}
	Vector3 Renderer::TraceRay(const Ray& ray, uint32_t depth, const HittableList& world)
	{
		if (depth == m_Settings.MaxDepth)
			m_PrimaryRays++;
		else
			m_SecondaryRays++;
		HitRecord record;
		if (depth <= 0)
			return Vector3(0.0f, 0.0f, 0.0f);

		if (world.Hit(ray, Interval(0.001f, infinity), record)) {
			// Normals shading
			if (isNormals)
				return (record.normal + 1.f) / 2.f;

			Ray scattered;
			Vector3 attenuation;
			if (record.mat->Scatter(ray, record, attenuation, scattered)) {
				return attenuation * TraceRay(scattered, depth - 1, world);
			}
			return Vector3(0, 0, 0);
		}
		if (useEnvMap) {
			float u = 0.5f + atan2(ray.GetDirection().z, ray.GetDirection().x) / (2 * pi);
			float v = 0.5f + asin(-ray.GetDirection().y) / pi;

			uint32_t x = u * (m_Width - 1);
			uint32_t y = v * (m_Height - 1);

			Vector3 color(0.f);
			int pixel = (y * m_Width + x) * 4;
			color.x = m_HDRData[pixel] / 255.f;
			color.y = m_HDRData[pixel + 1] / 255.f;
			color.z = m_HDRData[pixel + 2] / 255.f;

			return color;
		}
		// Sky box shading
		if (ray.GetDirection().y < 0)
			return Vector3(0.23f, 0.2f, 0.2f);

		Vector3 dir = ray.GetDirection();
		float a = 0.5f * (dir.y + 1.0f);
		Vector3 sky_color(0.5f, 0.7f, 1.0f);

		return (1.0f - a) * Vector3(1.0f) + a * sky_color;
	}
}

