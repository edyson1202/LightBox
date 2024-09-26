#pragma once

#include <memory>

#include "Vulkan/Image.h"
#include "Camera.h"
#include "HittableList.h"
#include "Math/Vector3.h"
#include "Scene.h"

namespace LightBox 
{
	struct Settings
	{
		bool useEnvMap = true;
		uint32_t MaxDepth = 10;
		bool Accumulate = true;
	};
	class CpuPathTracer 
	{
	public:
		CpuPathTracer(Device& device, Camera& camera, Scene& new_scene);

		void OnResize(uint32_t width, uint32_t height);
		void Render();

		std::shared_ptr<Image>& GetFinalImage() { return m_FinalImage; }
		void ResetFrameIndex() { m_FrameIndex = 1; }
		uint32_t GetFrameIndex() const { return m_FrameIndex; }
		Settings& GetSettings() { return m_Settings; }

		void SaveRenderToDisk() const;
	private:
		// TODO implement functions that correspond to ray-tracing shaders in the Vulkan rt pipeline, get rid of the HitPayload struct
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

	private:
		Device& m_Device;
		Camera& m_Camera;
		Scene& m_Scene;

		std::shared_ptr<Image> m_FinalImage;
		uint32_t* m_ImageData = nullptr;

		Vector3* m_AccumulationData = nullptr;
		uint32_t m_FrameIndex = 1;

		std::vector<uint32_t> m_ImageHorizontalIter;
		std::vector<uint32_t> m_ImageVerticalIter;

		Settings m_Settings;
	};
}