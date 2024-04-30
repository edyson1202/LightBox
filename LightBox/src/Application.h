#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <functional>

#include "Layer.h"
#include "Swapchain.h"
#include "Pipeline.h"
#include "Buffer.h"
#include "Renderer2.h"
#include "Device.h"

const uint32_t WIDTH = 1920;
const uint32_t HEIGHT = 1080;

struct GLFWwindow;

namespace LightBox {
	class Application {
	public:
		Application();
		~Application();

		void Run();
		void Init();
		void Shutdown();

		void FrameRender();
		void RecordRenderingCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex);

		static Application& Get();

		Device& GetDevice();

		GLFWwindow* GetWindowHandle() { return m_WindowHandle; };

		static void SubmitResourceFree(std::function<void()>&& func);
		
		void SetWindowResized(bool value) { bWindowResized = value; }

		Layer* m_Layer = nullptr;
	private:
		void InitImGui(GLFWwindow* window);
		float GetTime();

		void CreateImGuiDescriptorPool();
		void CreateCommandBuffers();
	private:

		uint32_t m_CurrentFrame = 0;
		uint32_t m_RendererFrame = 0;

		GLFWwindow* m_WindowHandle = nullptr;
		uint32_t m_Width = WIDTH;
		uint32_t m_Height = HEIGHT;
		const char* m_Name = "LightBox";
		bool bWindowResized = false;

		Device* m_Device;
		Pipeline* m_Pipeline;

		float m_TimeStep = 0.f;
		float m_FrameTime = 0.f;
		float m_LastFrameTime = 0.f;

		VkDescriptorPool m_ImGuiDescriptorPool = VK_NULL_HANDLE;

		std::vector<VkCommandBuffer> m_CommandBuffers;

	public:
	};
	// Implemented by CLIENT
	Application* CreateApplication(int argc, char** argv);
}
