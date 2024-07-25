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
#include "BVH_Node.h"

float resolution_scale = 10;
using namespace LightBox;
class ExampleLayer : public LightBox::Layer
{
public:
	ExampleLayer(Device& device)
		: m_Device(device), m_Camera(90.f, 0.1f, 100.f),
		m_CpuPathTracer(device, m_Camera, m_Scene),
		m_GpuRenderer(m_Device, m_Camera, m_Viewport),
		m_Viewport(device, m_GpuRenderer.GetRenderPass()),
		m_RTScene(device), m_VulkanScene(m_Device)
	{

		auto mesh_data_01 = m_CpuScene.LoadDataFromOBJ("C:/Users/Boris/Desktop/statue.obj");
		m_VulkanScene.LoadSceneFromRAM(m_CpuScene);

		m_GpuRenderer.SetScene(m_VulkanScene);
		{
			auto material_ground = new Lambertian(Vector3(0.65f, 0.65f, 0.5f));
			auto material_ground_01 = new Lambertian(Vector3(0.65f));
			auto material_center = new Lambertian(Vector3(0.7f, 0.3f, 0.3f));
			auto material_triangle = new Lambertian(Vector3(0.8f, 0.8f, 0.8f));
			auto material_left = new Dielectric(1.5f);
			auto material_right = new Metal(Vector3(0.8f, 0.6f, 0.4f), 0.1f);
			auto material_dragon = new Metal(Vector3(0.8f, 0.8f, 0.8f), 0.4f);

			//m_Scene.Add(std::make_shared<Sphere>(Vector3(1.5f, 0.f, -1.f), 0.5f, material_right));
			//m_Scene.Add(std::make_shared<Sphere>(Vector3(-0.078041, 0.502534, -0.867403), 0.01f, material_right));
			//m_Scene.Add(std::make_shared<Sphere>(Vector3(0, 0.f, -1.3f), 0.5f, material_center));
			//m_Scene.Add(std::make_shared<Sphere>(Vector3(-1.05f, 0.f, -1.f), 0.5f, material_left));
			//m_Scene.Add(std::make_shared<Sphere>(Vector3(-1.05f, 0.f, -1.f), -0.4f, material_left));
			m_Scene.Add(std::make_shared<Sphere>(Vector3(0, -100.5f, -1), 100, material_ground));

			//m_Scene.Add(std::make_shared<Triangle>(Vector3(0.f, 0.f, -1.f), Vector3(2.f, 0.f, -1.f), Vector3(1.f, 2.f, -2.f), material_triangle));

			//auto mesh_data = Mesh::GetMeshDataFromOBJ("C:/Users/Boris/Desktop/low_poly_dragon.obj");
			//m_Scene.Add(std::make_shared<Mesh>(mesh_data, material_dragon));
			m_Scene.Add(std::make_shared<Mesh>(mesh_data_01, material_right));
		} 
		// Ray tracing in one weekend
		/*
		{
			auto ground_material = new Lambertian(Vector3(0.5, 0.5, 0.5));
			m_Scene.Add(std::make_shared<Sphere>(Vector3(0, -1000, 0), 1000, ground_material));

			for (int a = -11; a < 11; a++) {
				for (int b = -11; b < 11; b++) {
					auto choose_mat = Random::GetRandomFloat();
					Vector3 center(a + 0.9 * Random::GetRandomFloat(), 0.2, b + 0.9 * Random::GetRandomFloat());

					if ((center - Vector3(4, 0.2, 0)).GetLength() > 0.9) {
						Material* sphere_material;

						if (choose_mat < 0.8) {
							// diffuse
							auto albedo = Random::RandomVector3(0, 1) * Random::RandomVector3(0, 1);
							sphere_material = new Lambertian(albedo);
							m_Scene.Add(std::make_shared<Sphere>(center, 0.2, sphere_material));
						}
						else if (choose_mat < 0.95) {
							// metal
							auto albedo = Random::RandomVector3(0.5, 1);
							auto fuzz = Random::GetRandomFloat(0, 0.5);
							sphere_material = new Metal(albedo, fuzz);
							m_Scene.Add(std::make_shared<Sphere>(center, 0.2, sphere_material));
						}
						else {
							// glass
							sphere_material = new Dielectric(1.5);
							m_Scene.Add(std::make_shared<Sphere>(center, 0.2, sphere_material));
						}
					}
				}
			}

			auto material1 = new Dielectric(1.5);
			m_Scene.Add(std::make_shared<Sphere>(Vector3(0, 1, 0), 1.0, material1));

			auto material2 = new Lambertian(Vector3(0.4, 0.2, 0.1));
			m_Scene.Add(std::make_shared<Sphere>(Vector3(-4, 1, 0), 1.0, material2));

			auto material3 = new Metal(Vector3(0.7, 0.6, 0.5), 0.0);
			m_Scene.Add(std::make_shared<Sphere>(Vector3(4, 1, 0), 1.0, material3));
		} */	
		
		//m_Scene = HittableList(std::make_shared<BVH_Node>(m_Scene));
	}
	virtual void OnUpdate(float ts) override {
		if (m_Camera.OnUpdate(ts))
			m_CpuPathTracer.ResetFrameIndex();
		if (Input::IsKeyDown(KeyCode::Z))
			m_IsRayTracing = !m_IsRayTracing;
	}
	virtual void OnUIRender(uint32_t current_frame) override {
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		ImGui::Text("Sample count: %d", m_CpuPathTracer.GetFrameIndex());
		ImGui::Checkbox("Render normals", &m_CpuPathTracer.isNormals);
		ImGui::Checkbox("Real time", &m_RealTime);
		ImGui::Checkbox("Use environment map", &m_CpuPathTracer.useEnvMap);
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
		float new_fov = m_Camera.GetFOV();
		ImGui::DragFloat("Field of view", &new_fov, 1.f, 1.f, 150.f);
		ImGui::InputFloat("Resolution scale", &resolution_scale);
		if (new_fov != m_Camera.GetFOV())
			m_Camera.SetFOV(new_fov);
		ImGui::End();


		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		m_ViewportWidth = (uint32_t)ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = (uint32_t)ImGui::GetContentRegionAvail().y;
		ImVec2 viewport_size = ImGui::GetContentRegionAvail();
		m_Camera.OnResize((uint32_t)(viewport_size.x / resolution_scale),
			(uint32_t)(viewport_size.y / resolution_scale));
		
		if (m_IsRayTracing) {
			auto image = m_CpuPathTracer.GetFinalImage();
			if (image)
				ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth() * resolution_scale, (float)image->GetHeight() * resolution_scale });
		}
		else {
			ImGui::Image(m_Viewport.GetDescriptorSets()[current_frame], viewport_size);
		}


		ImGui::End();
		ImGui::PopStyleVar();

		if (m_RealTime)
			Render();
	}
	virtual void Render() override{
		Timer timer;

		m_CpuPathTracer.OnResize((uint32_t)(m_ViewportWidth / resolution_scale),
			(uint32_t)(m_ViewportHeight / resolution_scale));
		//m_PathTracer.OnResize((uint32_t)(m_ViewportWidth), (uint32_t)(m_ViewportHeight));
		m_CpuPathTracer.Render();

		m_LastRenderTime = timer.ElapsedMillis();
	}
	virtual void RecordCommands(VkCommandBuffer& command_buffer, uint32_t imageIndex) override {
		if (m_IsRayTracing) {
			//m_PathTracer.RecordRenderingCommandBuffer(command_buffer, imageIndex);
		}
		else {
			m_GpuRenderer.RecordRenderingCommandBuffer(command_buffer, imageIndex);
		}
	}

private:
	Device& m_Device;

	Camera m_Camera;
	CpuPathTracer m_CpuPathTracer;
	GpuRenderer m_GpuRenderer;
	Viewport m_Viewport;

	HittableList m_Scene;

	VulkanRTScene m_RTScene;

	Scene m_CpuScene;
	VulkanScene m_VulkanScene;	

	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

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
