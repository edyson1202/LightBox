#pragma once
#include <algorithm>

#include "HostImage.h"
#include "Math/Vector3.h"

namespace LightBox
{
	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual Vector3 GetValue(float u, float v, const Vector3 point) const = 0;

	private:
		Vector3 m_Color01;
		Vector3 m_Color02;	
	};

	class SolidColor : public Texture
	{
	public:
		SolidColor(const Vector3 albedo)
			: m_Albedo(albedo) {}

		Vector3 GetValue(float u, float v, const Vector3 point) const override
		{
			return m_Albedo;
		}
	private:
		Vector3 m_Albedo;
	};
	class CheckerTexture : public Texture
	{
	public:
		CheckerTexture(Vector3 color_a, Vector3 color_b, float scale)
			: m_Color01(color_a), m_Color02(color_b), m_InvScale(1.f / scale) {}

		Vector3 GetValue(float u, float v, const Vector3 point) const override
		{
			int32_t x = int(std::floor(m_InvScale * point.x));
			int32_t y = int(std::floor(m_InvScale * point.y));
			int32_t z = int(std::floor(m_InvScale * point.z));

			bool isEven = (x + y + z) % 2 == 0;

			return isEven ? m_Color01 : m_Color02;
		}
	private:
		float m_InvScale;
		Vector3 m_Color01;
		Vector3 m_Color02;
	};
	class ImageTexture : public Texture
	{
	public:
		ImageTexture(const std::string& path, uint32_t channel_count = 3)
			: m_Image(path, channel_count) {}

		Vector3 GetValue(float u, float v, const Vector3 point) const override
		{
			if (m_Image.m_Height <= 0)
				return Vector3(0, 1, 1);

			u = std::clamp(u, 0.f, 1.f);
			v = 1.f - std::clamp(v, 0.f, 1.f);

			uint32_t x = u * (m_Image.m_Width - 1);
			uint32_t y = v * (m_Image.m_Height - 1);

			auto pixel = m_Image.GetPixelData(x, y);

			float color_scale = 1.f / 255.f;

			return Vector3(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);
		}

		Vector3 GetValueOnSphere(const Vector3& direction)
		{
			float u = 0.5f + atan2(direction.z, direction.x) / (2 * pi);
			float v = 0.5f + asin(-direction.y) / pi;

			uint32_t x = u * (m_Image.m_Width - 1);
			uint32_t y = v * (m_Image.m_Height - 1);

			return m_Image.GetPixelDataFloat((int)x, (int)y);
			return 2.f * m_Image.GetPixelData((int)x, (int)y);
		}

	private:
		HostImage m_Image;
	};
}
