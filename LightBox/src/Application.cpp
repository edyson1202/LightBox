#include "Application.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include <iostream>
#include <stdio.h> // fprintf, printf
#include <stdlib.h> // abort
#include <vector>
#include <assert.h>
#include <algorithm> // clamp
#include <array>

#include "Utils.h"
#include "Vertex.h"
#include "Image.h"
#include "VulkanUtils.h"

static ImGuiIO*					g_ImGuiIO;

// Per-frame-in-flight
static std::vector<std::vector<std::function<void()>>> s_ResourceFreeQueue;

static LightBox::Application* s_appInstance;

using namespace LightBox;

static void InitVulkan()
{
	s_ResourceFreeQueue.resize(MAX_FRAMES_IN_FLIGHT);
}
static void ShutdownVulkan()
{
	// Free resources in queue
	for (auto& queue : s_ResourceFreeQueue)
	{
		for (auto& func : queue)
			func();
	}
	s_ResourceFreeQueue.clear();
}
static void ShutdownImGui()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

namespace LightBox {
static void glfw_frame_buffer_resize_callback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->SetWindowResized(true);
}
Application::Application()
{
	s_appInstance = this;
	Init();
}
Application::~Application()
{
	Shutdown();
	s_appInstance = nullptr;
}
void Application::Init()
{
	// Setup GLFW window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) {
		std::cerr << "Count not initialize GLFW\n";
		return;
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
	m_WindowHandle = glfwCreateWindow(m_Width, m_Height, m_Name, NULL, NULL);
	glfwSetWindowUserPointer(m_WindowHandle, this);
	glfwSetFramebufferSizeCallback(m_WindowHandle, glfw_frame_buffer_resize_callback);

	// Setup Vulkan
	InitVulkan();

	m_Device = new Device(m_WindowHandle);

	CreateImGuiDescriptorPool();
	CreateCommandBuffers();
	InitImGui(m_WindowHandle);
	//m_Pipeline = new Pipeline(*m_Device);
}

void Application::Shutdown()
{


	ShutdownImGui();

	vkDestroyDescriptorPool(m_Device->GetDevice(), m_ImGuiDescriptorPool, m_Device->GetAllocator());

	ShutdownVulkan();
	glfwDestroyWindow(m_WindowHandle);
	glfwTerminate();
}	
void Application::Run()
{
	// Main Loop
	while (!glfwWindowShouldClose(m_WindowHandle)) {
		glfwPollEvents();

		if (m_Layer != nullptr) {
			m_Layer->OnUpdate(m_TimeStep);
		}

		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Code that enables docking to the main window
		{
			static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

			// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
			// because it would be confusing to have two docking targets within each others.
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

			// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
			// and handle the pass-thru hole, so we ask Begin() to not render a background.
			if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
				window_flags |= ImGuiWindowFlags_NoBackground;

			// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
			// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
			// all active windows docked into it will lose their parent and become undocked.
			// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
			// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("DockSpace Demo", nullptr, window_flags);
			ImGui::PopStyleVar();

			ImGui::PopStyleVar(2);

			// Submit the DockSpace
			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			{
				ImGuiID dockspace_id = ImGui::GetID("VulkanAppDockspace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			}

			ImGui::End();
		}
		if (m_Layer != nullptr) {
			m_Layer->OnUIRender(m_RendererFrame);
		}
		// RENDERING
		ImGui::Render();
		// this function renders and presents	
		// Update and Render additional Platform Windows
		if (g_ImGuiIO->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
		FrameRender();

		float time = GetTime();
		m_FrameTime = time - m_LastFrameTime;
		m_TimeStep = std::min(m_FrameTime, 0.0333f);
		m_LastFrameTime = time;
	}
	vkDeviceWaitIdle(m_Device->GetDevice());
}
void Application::FrameRender() {
	VkDevice device = m_Device->GetDevice();
	VkQueue& graphics_queue = m_Device->GetGraphicsQueue();
	Swapchain& swapchain = m_Device->GetSwapchain();

	vkWaitForFences(device, 1, &swapchain.inFlightFence[m_CurrentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult err = vkAcquireNextImageKHR(device, *swapchain.GetSwapchain(), UINT64_MAX, swapchain.imageAvailableSemaphore[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);
	if (err == VK_ERROR_OUT_OF_DATE_KHR) {
		swapchain.RecreateSwapchain();
		return;
	}
	else if (err != VK_SUCCESS && err != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Failed to acquire swapchain image!");
	}
	vkResetFences(device, 1, &swapchain.inFlightFence[m_CurrentFrame]);

	vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);
	RecordRenderingCommandBuffer(m_CommandBuffers[m_CurrentFrame], imageIndex);


	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { swapchain.imageAvailableSemaphore[m_CurrentFrame] };
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = waitSemaphores;
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.pWaitDstStageMask = waitStages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];
	VkSemaphore signalSemaphores[] = { swapchain.renderFinishedSemaphore[m_CurrentFrame] };
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signalSemaphores;

	err = vkQueueSubmit(graphics_queue, 1, &submit_info, swapchain.inFlightFence[m_CurrentFrame]);
	check_vk_result(err);

	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swapchains[] = { *swapchain.GetSwapchain() };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapchain.GetSwapchain();
	present_info.pImageIndices = &imageIndex;
	present_info.pResults = nullptr;

	err = vkQueuePresentKHR(graphics_queue, &present_info);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
		swapchain.RecreateSwapchain();
	}
	else if (err != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swapchain image!");
	}

	//for (auto& func : s_ResourceFreeQueue[g_CurrentFrame])
	//	func();
	//s_ResourceFreeQueue[g_CurrentFrame].clear();

	m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
void Application::RecordRenderingCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex)
{
	Swapchain& swapchain = m_Device->GetSwapchain();

	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = 0;
	begin_info.pInheritanceInfo = nullptr;

	VkResult err = vkBeginCommandBuffer(commandBuffer, &begin_info);
	check_vk_result(err);

	VkRenderPassBeginInfo renderPass_info{};
	renderPass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPass_info.renderPass = *swapchain.GetRenderPass();
	renderPass_info.framebuffer = swapchain.m_SwapchainFramebuffers[imageIndex];
	renderPass_info.renderArea.offset = { 0, 0 };
	renderPass_info.renderArea.extent = swapchain.GetExtent();
	VkClearValue clearColor{ {{0.2f, 0.2f, 0.2f, 1.f}} };
	renderPass_info.clearValueCount = 1;
	renderPass_info.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPass_info, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapchain.GetExtent().width);
	viewport.height = static_cast<float>(swapchain.GetExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapchain.GetExtent();
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	// Record dear imgui primitives into command buffer
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

	vkCmdEndRenderPass(commandBuffer);

	m_Layer->RecordCommands(commandBuffer, imageIndex);

	err = vkEndCommandBuffer(commandBuffer);

	m_RendererFrame = imageIndex;
	check_vk_result(err);
}
void Application::InitImGui(GLFWwindow* window)
{
	// Setup Dear ImGui context
	// This initializes the core structures of ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	g_ImGuiIO = &ImGui::GetIO(); (void)g_ImGuiIO;
	g_ImGuiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	g_ImGuiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	g_ImGuiIO->ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	g_ImGuiIO->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (g_ImGuiIO->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Initializig ImGui for Vulkan
	ImGui_ImplGlfw_InitForVulkan(window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = m_Device->GetInstance();
	init_info.PhysicalDevice = m_Device->GetPhysicalDevice();
	init_info.Device = m_Device->GetDevice();
	init_info.QueueFamily = m_Device->GetGraphicsQueueFamilyIndex();
	init_info.Queue = m_Device->GetGraphicsQueue();
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = m_ImGuiDescriptorPool;
	init_info.Subpass = 0;

	init_info.MinImageCount = m_Device->GetSwapchain().GetMinImageCount();
	init_info.ImageCount = m_Device->GetSwapchain().GetImageCount();

	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = m_Device->GetAllocator();
	init_info.CheckVkResultFn = check_vk_result;

	ImGui_ImplVulkan_Init(&init_info, *(m_Device->GetSwapchain().GetRenderPass()));
}
float Application::GetTime()
{
	return (float)glfwGetTime();
}
Device& Application::GetDevice() {
	return *m_Device;
}
void Application::CreateImGuiDescriptorPool() {
	// Create descriptor pool for IMGUI
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	VkResult err = vkCreateDescriptorPool(m_Device->GetDevice(), &pool_info, m_Device->GetAllocator(), &m_ImGuiDescriptorPool);
	check_vk_result(err);
}
void Application::CreateCommandBuffers()
{
	//m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	m_CommandBuffers.resize(m_Device->GetSwapchain().GetImageCount());

	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = m_Device->GetCommandPool();
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	//alloc_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
	alloc_info.commandBufferCount = m_Device->GetSwapchain().GetImageCount();

	VkResult err = vkAllocateCommandBuffers(m_Device->GetDevice(), &alloc_info, m_CommandBuffers.data());
	check_vk_result(err);
}
//void Application::SubmitResourceFree(std::function<void()>&& func) {
//	s_ResourceFreeQueue[g_CurrentFrame].emplace_back(func);
//}
Application& Application::Get() {
	return *s_appInstance;
}
}