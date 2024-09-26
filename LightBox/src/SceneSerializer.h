#pragma once
#include "Camera.h"
#include "Scene.h"
#include "yaml-cpp/emitter.h"

namespace LightBox
{
	class SceneSerializer
	{
	public:
		SceneSerializer(Scene& scene, Camera& camera)
			: m_Scene(scene), m_Camera(camera) {}
		// Current implementation only serializes camera data
		void Serialize(const std::string& filepath) const;
		bool Deserialize(const std::string& filepath) const;

	private:
		Scene& m_Scene;
		Camera& m_Camera;
	};
}
