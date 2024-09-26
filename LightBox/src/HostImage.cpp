#include "HostImage.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>

#include "Math/Vector3.h"


namespace LightBox
{
	HostImage::HostImage(const std::string& path, uint32_t bytes_per_pixel)
		: m_bytes_per_pixel(bytes_per_pixel)
	{
		Load(path);
	}

	const unsigned char* HostImage::GetPixelData(uint32_t x, uint32_t y) const
	{
		static unsigned char magenta[] = { 255, 0, 255 };
		if (m_bImageData == nullptr)
			return magenta;

		return m_bImageData + (y * m_bytes_per_pixel * m_Width) + m_bytes_per_pixel * x;
	}

	Vector3 HostImage::GetPixelData(int x, int y) const
	{
		int pixel = (y * m_Width * m_bytes_per_pixel) + x * m_bytes_per_pixel;
		float color_scale = 1.f / 255.f;
		return color_scale * Vector3(m_bImageData[pixel], m_bImageData[pixel + 1], m_bImageData[pixel + 2]);
	}

	Vector3 HostImage::GetPixelDataFloat(int x, int y) const
	{
		int pixel = (y * m_Width * m_bytes_per_pixel) + x * m_bytes_per_pixel;
		return Vector3(m_fImageData[pixel], m_fImageData[pixel + 1], m_fImageData[pixel + 2]);
	}


	bool HostImage::Load(const std::string& path)
	{
		int width, height, channelCount;
		m_fImageData = stbi_loadf(path.c_str(), &width, &height, &channelCount, m_bytes_per_pixel);

		if (!m_fImageData)
			throw std::runtime_error("Failed to load texture image!\n");

		m_Width = width;
		m_Height = height;

		ConvertToBytes();
	}

	unsigned char HostImage::FloatToByte(float value)
	{
		if (value <= 0)
			return 0;
		if (value >= 1.f)
			return 255;

		return static_cast<unsigned char>(256.f * value);
	}

	void HostImage::ConvertToBytes()
	{
		int bytes_count = m_Width * m_Height * m_bytes_per_pixel;
		m_bImageData = new unsigned char[bytes_count];

		auto bptr = m_bImageData;
		auto fptr = m_fImageData;

		for (int32_t i = 0; i < bytes_count; i++, bptr++, fptr++)
			*bptr = FloatToByte(*fptr);
	}
}
