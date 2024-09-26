#pragma once
#include "Scene.h"

namespace LightBox
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel(Scene& scene)
			: m_Scene(scene) {}


		void OnImGuiRender();
	private:
		Scene& m_Scene;
	};
}

