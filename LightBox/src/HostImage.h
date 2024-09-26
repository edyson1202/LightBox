#pragma once

#include <algorithm>
#include <string>

#include "Math/Vector3.h"

namespace LightBox
{
	class HostImage
	{
	public:
		HostImage(const std::string& path, uint32_t bytes_per_pixel);

		const unsigned char* GetPixelData(uint32_t x, uint32_t y) const;

		Vector3 GetPixelData(int x, int y) const;

		Vector3 GetPixelDataFloat(int x, int y) const;

	private:
		bool Load(const std::string& path);

		static unsigned char FloatToByte(float value);

		void ConvertToBytes();

	public:
		float* m_fImageData = nullptr;
		unsigned char* m_bImageData = nullptr;
		const uint32_t m_bytes_per_pixel = 3;
		int m_Width = 0;
		int m_Height = 0;
	};

}
