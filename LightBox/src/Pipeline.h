#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"
#include "Swapchain.h"

namespace LightBox {
	class Renderer2;

	class Application;
	class Pipeline final
	{
	public:
		Pipeline(Device& device, Renderer2* renderer);
		Pipeline(Device& device);
		~Pipeline();

		VkPipeline GetPipeline() { return m_GraphicsPipeline; }
		VkPipelineLayout GetPipelineLayout() { return m_PipelineLayout; }

		static uint32_t* ReadShaderCode(const char* filename, uint32_t& codeSize);
		static VkShaderModule CreateShaderModule(Device& device, uint32_t* code, uint32_t codeSize);
	private:
		void CreateGraphicsPipeline();
	private:
		VkPipeline m_GraphicsPipeline;
		VkPipelineLayout m_PipelineLayout;

		Device& m_Device;
		VkAllocationCallbacks* m_Allocator;
		Renderer2* m_Renderer = nullptr;
	};
}
