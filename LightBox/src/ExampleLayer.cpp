#include "ExampleLayer.h"

namespace LightBox {
	ExampleLayer::ExampleLayer(Device& device)
		: m_Device(device) {
		m_Camera = new Camera2(70.f, 0.1f, 100.f);
		m_Renderer2 = new Renderer2(m_Device, *m_Camera);
	}
	void ExampleLayer::OnUpdate(float ts) {
		std::cout << "time: " << ts << "\n";

	}
	void ExampleLayer::OnUIRender() {
		//ImGui::Begin("Settings");
		//ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		//if (ImGui::Button("Render")) {
		//	Render();
		//}
		//ImGui::End();

		//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		//ImGui::Begin("Viewport");

		//m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		//m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		//auto image = m_Renderer.GetFinalImage();
		//if (image)
		//	ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() });

		//ImGui::End();
		//ImGui::PopStyleVar();

		Render();
	}
	void ExampleLayer::Render() {
		Timer timer;

		//m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		//m_Renderer.Render();

		//m_LastRenderTime = timer.ElapsedMillis();

		m_Renderer2->FrameRender();
	}
}