#pragma once

#include <memory>

#include "Vulkan/Image.h"
#include "Camera.h"
#include "HittableList.h"
#include "Math/Vector3.h"

namespace LightBox 
{
	// CPU Path Tracer
	class CpuPathTracer 
	{
	public:
		struct Settings
		{
			uint32_t MaxDepth = 10;
			bool Accumulate = true;
		};
	public:
		CpuPathTracer(Device& device, Camera& camera, HittableList& scene);

		void OnResize(uint32_t width, uint32_t height);
		void Render();

		std::shared_ptr<Image> GetFinalImage() const { return m_FinalImage; }
		void ResetFrameIndex() { m_FrameIndex = 1; }
		uint32_t GetFrameIndex() { return m_FrameIndex; }
		Settings& GetSettings() { return m_Settings; }

		void SaveRenderToDisk() const;
	private:
		struct HitPayload
		{
			float hit_distance;
			Vector3 world_pos;
			Vector3 world_normal;

			uint32_t object_index;
		};

		Vector3 PerPixel(uint32_t x, uint32_t y);
		Vector3 RayGen(uint32_t x, uint32_t y);
		HitPayload TraceRay(const Ray& ray);
		HitPayload ClosestHit(const Ray& ray, float hit_distance, uint32_t object_index);
		HitPayload Miss(const Ray& ray);
		Vector3 TraceRay(const Ray& ray, uint32_t depth, const HittableList& world);
	public:
		bool useEnvMap = false;
		bool isNormals = false;
	private:
		std::shared_ptr<Image> m_FinalImage;
		uint32_t* m_ImageData = nullptr;

		Vector3* m_AccumulationData = nullptr;
		uint32_t m_FrameIndex = 1;

		Device& m_Device;
		Camera& m_Camera;

		HittableList& m_World;

		Image* m_Background = nullptr;

		std::vector<uint32_t> m_ImageHorizontalIter;
		std::vector<uint32_t> m_ImageVerticalIter;

		uint8_t* m_HDRData = nullptr;
		int m_Width;
		int m_Height;
		int m_Channels;

		Settings m_Settings;

		uint32_t m_PrimaryRays = 0;
		uint32_t m_SecondaryRays = 0;
	};
}