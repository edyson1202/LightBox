#pragma once
#include "Application.h"
#include "Layer.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include <memory>

#include "Vulkan/Image.h"
#include "EntryPoint.h"
#include "Math/Random.h"
#include "Timer.h"
#include "Math/Vector3.h"
#include "CpuPathTracer.h"
#include "Camera.h"
#include "Viewport.h"
#include "Input/Input.h"
#include "Sphere.h"
#include "Mesh.h"
#include "Material.h"
#include "Vulkan/VulkanRT/GpuPathTracer.h"
#include "Vulkan/VulkanRT/VulkanRTScene.h"
#include "SceneHierarchyPanel.h"
#include "SceneSerializer.h"

float resolution_scale = 5;
using namespace LightBox;

enum EDevice {
	CPU,
	GPU
};

class ExampleLayer : public LightBox::Layer
{
public:
	ExampleLayer(Device& device)
		: m_Device(device),
		m_Camera(90.f, 0.1f, 100.f),
		//m_GpuRenderer(m_Device, m_Camera, m_Viewport),
		//m_Viewport(device, m_GpuRenderer.GetRenderPass()),
		m_VulkanScene(m_Device),
		m_RTScene(device),
		m_GpuPathTracer(device),
		m_CpuPathTracer(device, m_Camera, m_CpuScene),
		m_OutlinerPanel(m_CpuScene)
	{
		// DESERIALIZATION
		SceneSerializer scene_serializer(m_CpuScene, m_Camera);

		if (scene_serializer.Deserialize("assets/scenes/Example.yml"))
			std::cout << "Deserialization successful!\n";
		else
			std::cout << "Deserialization failed!\n";

		// LOADING SCENE
		m_CpuScene.LoadDataFromObj("resources/scene_01.obj");

		m_CpuScene.LoadEnvMap("resources/env_map_03.png");

		m_RTScene.UploadSceneToGPU(m_CpuScene.GetVertexBuffer(), m_CpuScene.GetIndexBuffer(), m_CpuScene.GetObjDescriptions());


		m_GpuPathTracer.Init(&m_RTScene, &m_Camera);

		//m_GpuRenderer.SetScene(m_VulkanScene);
		{
			auto material_ground = new Lambertian(Vector3(0.65f, 0.65f, 0.5f));

			m_Scene.Add(std::make_shared<Sphere>(Vector3(0, -100.f, -1), 100, material_ground));
		} 
	}

	~ExampleLayer() override
	{
		SceneSerializer scene_serializer(m_CpuScene, m_Camera);

		scene_serializer.Serialize("assets/scenes/Example.yml");
	}

	void OnUpdate(float ts) override {
		if (m_Camera.OnUpdate(ts))
			m_CpuPathTracer.ResetFrameIndex();
		if (Input::IsKeyDown(KeyCode::Z))
			m_IsRayTracing = !m_IsRayTracing;
	}

	void OnUIRender(uint32_t current_frame) override {
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		ImGui::Text("Sample count: %d", m_CpuPathTracer.GetFrameIndex());
		ImGui::Checkbox("CPU Real time rendering", &m_RealTime);

	
		const char* items[] = { "CPU", "GPU" };

		ImGui::Combo("Device", (int*) & m_HardwareDevice, items, IM_ARRAYSIZE(items));

		ImGui::Checkbox("Use environment map", &m_CpuPathTracer.GetSettings().useEnvMap);
		if (ImGui::Button("Render")) {
			Render();
		}
		if (ImGui::Button("Save render")) {
			m_CpuPathTracer.SaveRenderToDisk();
		}
		if (ImGui::Button("Reset")) {
			m_CpuPathTracer.ResetFrameIndex();
		}
		ImGui::Checkbox("Accumulate", &m_CpuPathTracer.GetSettings().Accumulate);
		ImGui::End();

		ImGui::Begin("Scene");
		Sphere* sphere = static_cast<Sphere*>(m_Scene.m_Objects[0].get());
		Metal* metal_mat = (Metal*)sphere->m_Mat;

		ImGui::DragFloat3("Position", (float*)&sphere->m_Center, 0.1f);
		ImGui::DragFloat("Radius", &sphere->m_Radius, 0.1f, 0.1f, 2.5f);
		ImGui::ColorEdit3("Albedo", &metal_mat->m_Albedo.x);
		ImGui::DragFloat("Roughness", &metal_mat->m_Fuzz, 0.002f, 0.f, 1.f);
		ImGui::End();

		ImGui::Begin("Camera");
		float new_fov = m_Camera.GetFov();
		ImGui::DragFloat("Field of view", &new_fov, 1.f, 1.f, 150.f);
		ImGui::InputFloat("Resolution scale", &resolution_scale);
		ImGui::DragFloat3("Position", (float*)&m_Camera.GetPosition(), 0.1f, -10000.f, 1000.f);
		m_Camera.SetFov(new_fov);
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		m_ViewportWidth = (uint32_t)ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = (uint32_t)ImGui::GetContentRegionAvail().y;
		ImVec2 viewport_size = ImGui::GetContentRegionAvail();
		m_Camera.OnResize((uint32_t)(m_ViewportWidth / resolution_scale),
		                  (uint32_t)(m_ViewportHeight / resolution_scale));
		
		if (m_IsRayTracing) {
			if (m_HardwareDevice == EDevice::GPU)
			{
				std::shared_ptr<Image>& image = m_GpuPathTracer.GetFinalImage();

				ImGui::Image(image->GetDescriptorSet(), 
					{ (float)image->GetWidth(), (float)image->GetHeight() });
			}
			else
			{
				std::shared_ptr<Image>& image = m_CpuPathTracer.GetFinalImage();

				if (image)
					ImGui::Image(image->GetDescriptorSet(),
						{ (float)image->GetWidth() * resolution_scale, (float)image->GetHeight() * resolution_scale });
			}
		}
		else {
			//ImGui::Image(m_Viewport.GetDescriptorSets()[current_frame], viewport_size);
		}

		ImGui::End();
		ImGui::PopStyleVar();

		m_OutlinerPanel.OnImGuiRender();

		m_GpuPathTracer.OnResize(m_ViewportWidth, m_ViewportHeight);

		if (m_RealTime && m_HardwareDevice == EDevice::CPU)
			Render();
	}
	void Render() override
	{
		Timer timer;

		m_CpuPathTracer.OnResize((uint32_t)(m_ViewportWidth / resolution_scale),
			(uint32_t)(m_ViewportHeight / resolution_scale));

		m_CpuPathTracer.Render();

		m_LastRenderTime = timer.ElapsedMillis();
	}
	void RecordCommands(VkCommandBuffer& command_buffer, uint32_t imageIndex) override
	{
		if (m_IsRayTracing && m_HardwareDevice == EDevice::GPU) {
			//m_GpuPathTracer.OnResize((uint32_t)(m_ViewportWidth), (uint32_t)(m_ViewportHeight));

			m_GpuPathTracer.RayTrace(command_buffer, imageIndex);
		}
		else {
			//m_GpuRenderer.RecordRenderingCommandBuffer(command_buffer, imageIndex);
		}
	}

private:
	Device& m_Device;

	Camera m_Camera;
	//GpuRenderer m_GpuRenderer;
	//Viewport m_Viewport;

	Scene m_CpuScene;
	VulkanScene m_VulkanScene;
	HittableList m_Scene;

	VulkanRTScene m_RTScene;
	GpuPathTracer m_GpuPathTracer;

	CpuPathTracer m_CpuPathTracer;

	SceneHierarchyPanel m_OutlinerPanel;

	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

	EDevice m_HardwareDevice = EDevice::CPU;
	float m_LastRenderTime = 0.0f;

	bool m_IsRayTracing = true;
	bool m_RealTime = false;
};

LightBox::Application* LightBox::CreateApplication(int argc, char** argv)
{
	LightBox::Application* app = new LightBox::Application();
	app->m_Layer = new ExampleLayer(app->GetDevice());
	app->m_Layer->OnAttach();

	return app;
}
