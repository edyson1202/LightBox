#include "SceneHierarchyPanel.h"

#include "imgui.h"

namespace  LightBox
{
	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Outliner");

		const auto& meshes = m_Scene.GetMeshes();

		for (const auto& mesh : meshes)
		{
			ImGui::Text(mesh->m_Name.c_str());
		}

		ImGui::End();
	}
}
