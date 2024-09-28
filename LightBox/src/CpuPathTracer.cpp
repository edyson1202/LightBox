#include "CpuPathTracer.h"

#include <stb_image.h>

#include <iostream>
#include <execution>

#include "Sphere.h"
#include "Math/Random.h"

namespace LightBox
{
	static Vector3 GetDefaultSkyValue(const Ray& ray)
	{
		if (ray.GetDirection().y < 0)
			return Vector3(0.23f, 0.2f, 0.2f);

		Vector3 dir = ray.GetDirection();
		float a = 0.5f * (dir.y + 1.0f);
		Vector3 sky_color(0.5f, 0.7f, 1.0f);

		return (1.0f - a) * Vector3(1.0f) + a * sky_color;
	}
	static void WriteImageToPPMFile(const uint32_t image_width, const uint32_t image_height, const uint32_t* image_data)
	{
		std::ofstream image_file("image.ppm");

		image_file << "P3\n" << image_width << ' ' << image_height << "\n255\n";

		for (int32_t y = 0; y < image_height; y++)
		{
			for (int32_t x = 0; x < image_width; x++)
			{
				uint32_t r = image_data[y * image_width + x] & 0x0000ff;
				uint32_t g = (image_data[y * image_width + x] & 0x00ff00) >> 8;
				uint32_t b = (image_data[y * image_width + x] & 0xff0000) >> 16;
				image_file << r << ' ' << g << ' ' << b << '\n';
			}
		}

		image_file.close();
	}
	static float LinearToGamma(const float linear_component)
	{
		if (linear_component > 0)
			return sqrt(linear_component);

		return 0;
	}
	// This is used for anti-aliasing
	Vector3 pixel_sample_square()
	{
		// Returns a random point in a square centered at the pixel's origin.
		float px = -0.5f + Random::Float();
		float py = -0.5f + Random::Float();
	
		return Vector3(px, py, 0.f);
	}
	CpuPathTracer::CpuPathTracer(Device& device, Camera& camera, Scene& new_scene)
		: m_Device(device), m_Camera(camera), m_Scene(new_scene)
	{
		
	}
	void CpuPathTracer::OnResize(uint32_t width, uint32_t height)
	{
		if (m_FinalImage) {
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
	void CpuPathTracer::Render() 
	{
		if (m_FrameIndex == 1)
			memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(Vector3));

#define MT 0
#if MT
		std::for_each(std::execution::par, m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(),
			[this](uint32_t y)
			{
				std::for_each(std::execution::par, m_ImageHorizontalIter.begin(), m_ImageHorizontalIter.end(),
				[this, y](uint32_t x)
					{
						uint32_t m_ImageWidth = m_FinalImage->GetWidth();
						Vector3 accumulated_color = PerPixel(x, y);

						accumulated_color.x = LinearToGamma(accumulated_color.x);
						accumulated_color.y = LinearToGamma(accumulated_color.y);
						accumulated_color.z = LinearToGamma(accumulated_color.z);

						accumulated_color = 255.0f * accumulated_color.Clamp(Vector3(0.f), Vector3(1.f));

						m_ImageData[y * m_ImageWidth + x] = 0xff000000;
						m_ImageData[y * m_ImageWidth + x] |= (uint32_t)accumulated_color.x;
						m_ImageData[y * m_ImageWidth + x] |= ((uint32_t)accumulated_color.y) << 8;
						m_ImageData[y * m_ImageWidth + x] |= ((uint32_t)accumulated_color.z) << 16;
					});
			});
#else
		for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
			for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) 
			{
				uint32_t m_ImageWidth = m_FinalImage->GetWidth();
				Vector3 accumulated_color = PerPixel(x, y);

				accumulated_color.x = LinearToGamma(accumulated_color.x);
				accumulated_color.y = LinearToGamma(accumulated_color.y);
				accumulated_color.z = LinearToGamma(accumulated_color.z);

				accumulated_color = 255.0f * accumulated_color.Clamp(Vector3(0.f), Vector3(1.f));

				m_ImageData[y * m_ImageWidth + x] = 0xff000000;
				m_ImageData[y * m_ImageWidth + x] |= (uint32_t)accumulated_color.x;
				m_ImageData[y * m_ImageWidth + x] |= ((uint32_t)accumulated_color.y) << 8;
				m_ImageData[y * m_ImageWidth + x] |= ((uint32_t)accumulated_color.z) << 16;
			}
		}
#endif
		m_FinalImage->SetData(m_ImageData);

		m_Settings.Accumulate ? m_FrameIndex++ : m_FrameIndex = 1;
	}

	Vector3 CpuPathTracer::PerPixel(uint32_t x, uint32_t y)
	{
		uint32_t m_ImageWidth = m_FinalImage->GetWidth();

		Ray ray(m_Camera.GetPosition(),
			m_Camera.GetRayDirections()[x + y * m_ImageWidth] + 0.0016f * pixel_sample_square());

		Vector3 color = TraceRay(ray, m_Settings.MaxDepth, m_Scene.GetHittableList());

		m_AccumulationData[y * m_ImageWidth + x] += color;

		return m_AccumulationData[y * m_ImageWidth + x] / (float)m_FrameIndex;
	}

	Vector3 CpuPathTracer::TraceRay(const Ray& ray, uint32_t depth, const HittableList& world)
	{
		HitRecord record;
		if (depth <= 0)
			return {0.f};

		if (!world.Hit(ray, Interval(0.001f, infinity), record))
			return m_Settings.useEnvMap ? m_Scene.m_EnvMap->GetValueOnSphere(ray.GetDirection()) : GetDefaultSkyValue(ray);

		Ray scattered;
		Vector3 attenuation;
		Vector3 color_from_emission = record.mat->Emitted(record.u, record.v, record.point);

		if (!record.mat->Scatter(ray, record, attenuation, scattered))
			return color_from_emission;

		Vector3 color_from_scatter = attenuation * TraceRay(scattered, depth - 1, world);

		return color_from_emission + color_from_scatter;
	}

	void CpuPathTracer::SaveRenderToDisk() const
	{
		WriteImageToPPMFile(m_FinalImage->GetWidth(), m_FinalImage->GetHeight(), m_ImageData);
	}
}

