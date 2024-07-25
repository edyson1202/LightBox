#pragma once

#include <vulkan/vulkan.h>

#include "Device.h"
#include "Swapchain.h"

namespace LightBox {
	class GpuRenderer;

	class Application;
	class Pipeline final
	{
	public:
		Pipeline(Device& device, GpuRenderer* renderer);
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
		GpuRenderer* m_Renderer = nullptr;
	};
}
