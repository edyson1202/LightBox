#include "Pipeline.h"

#include <fstream>
#include <iostream>
#include <cmath>
#include <limits>
#include <memory>

#include "VulkanUtils.h"
#include "Application.h"
#include "Vertex.h"

namespace LightBox {
	Pipeline::Pipeline(Device& device, GpuRenderer* renderer)
		: m_Device(device), m_Allocator(device.GetAllocator()), m_Renderer(renderer) {
		CreateGraphicsPipeline();
	}
	Pipeline::Pipeline(Device& device)
		: m_Device(device), m_Allocator(device.GetAllocator())
	{
		CreateGraphicsPipeline();
	}
	;
	Pipeline::~Pipeline() {
		vkDestroyPipelineLayout(m_Device.GetDevice(), m_PipelineLayout, m_Allocator);
		vkDestroyPipeline(m_Device.GetDevice(), m_GraphicsPipeline, m_Allocator);
	}
	void Pipeline::CreateGraphicsPipeline()
	{
		VkDevice& device = m_Device.GetDevice();

		static const char* vert_shader_path = "src/shaders/raster.vert.spv";
		static const char* frag_shader_path = "src/shaders/raster.frag.spv";

		uint32_t vert_code_size;
		uint32_t frag_code_size;
		uint32_t* vertCode = ReadShaderCode(vert_shader_path, vert_code_size);
		uint32_t* fragCode = ReadShaderCode(frag_shader_path, frag_code_size);

		VkShaderModule vert_shader_module = CreateShaderModule(m_Device, vertCode, vert_code_size);
		VkShaderModule frag_shader_module = CreateShaderModule(m_Device, fragCode, frag_code_size);

		VkPipelineShaderStageCreateInfo vert_shader_info{};
		vert_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert_shader_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vert_shader_info.module = vert_shader_module;
		vert_shader_info.pName = "main";

		VkPipelineShaderStageCreateInfo frag_shader_info{};
		frag_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_info.module = frag_shader_module;
		frag_shader_info.pName = "main";

		VkPipelineShaderStageCreateInfo shader_stages[2] = { vert_shader_info, frag_shader_info };

		VkPipelineVertexInputStateCreateInfo vertex_input_info{};
		vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		if (m_Renderer) {
			auto bindingDescriptions = Vertex::GetBindingDescriptions();
			auto attributeDescriptions = Vertex::GetAttributeDescriptions();
			vertex_input_info.vertexBindingDescriptionCount = 1;
			vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
			std::cout << vertex_input_info.vertexBindingDescriptionCount << " "
				<< vertex_input_info.vertexAttributeDescriptionCount;
			vertex_input_info.pVertexBindingDescriptions = &bindingDescriptions;
			vertex_input_info.pVertexAttributeDescriptions = attributeDescriptions.data();
		}
		else {
			vertex_input_info.vertexBindingDescriptionCount = 0;
			vertex_input_info.vertexAttributeDescriptionCount = 0;
			vertex_input_info.pVertexBindingDescriptions = nullptr;
			vertex_input_info.pVertexAttributeDescriptions = nullptr;
		}



		// INPUT ASSEMBLY
		VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
		input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly_info.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)m_Device.GetSwapchain().GetExtent().width;
		viewport.height = (float)m_Device.GetSwapchain().GetExtent().height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_Device.GetSwapchain().GetExtent();

		VkPipelineViewportStateCreateInfo viewport_state_info{};
		viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state_info.viewportCount = 1;
		viewport_state_info.pViewports = &viewport;
		viewport_state_info.scissorCount = 1;
		viewport_state_info.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.f;
		rasterizer.depthBiasClamp = 0.f;
		rasterizer.depthBiasSlopeFactor = 0.f;

		VkPipelineMultisampleStateCreateInfo multisample_state_info{};
		multisample_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state_info.sampleShadingEnable = VK_FALSE;
		multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state_info.minSampleShading = 1.f;
		multisample_state_info.pSampleMask = nullptr;
		multisample_state_info.alphaToCoverageEnable = VK_FALSE;
		multisample_state_info.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState color_blend_attachment{};
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment.blendEnable = VK_FALSE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo color_blending_info{};
		color_blending_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blending_info.logicOpEnable = VK_FALSE;
		color_blending_info.logicOp = VK_LOGIC_OP_COPY; // Optional
		color_blending_info.attachmentCount = 1;
		color_blending_info.pAttachments = &color_blend_attachment;
		color_blending_info.blendConstants[0] = 0.0f; // Optional
		color_blending_info.blendConstants[1] = 0.0f; // Optional
		color_blending_info.blendConstants[2] = 0.0f; // Optional
		color_blending_info.blendConstants[3] = 0.0f; // Optional

		VkDynamicState dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
		dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateInfo.dynamicStateCount = 2;
		dynamicStateInfo.pDynamicStates = dynamicStates;

		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		if (m_Renderer) {
			pipeline_layout_info.setLayoutCount = 1;
			pipeline_layout_info.pSetLayouts = m_Renderer->GetDescriptorSetLayout();
		}
		else {
			pipeline_layout_info.setLayoutCount = 0;
			pipeline_layout_info.pSetLayouts = nullptr;
		}

		pipeline_layout_info.pushConstantRangeCount = 0;
		pipeline_layout_info.pPushConstantRanges = nullptr;

		VkResult err = vkCreatePipelineLayout(device, &pipeline_layout_info, m_Allocator, &m_PipelineLayout);
		check_vk_result(err);

		VkGraphicsPipelineCreateInfo pipeline_info{};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = 2;
		pipeline_info.pStages = shader_stages;
		pipeline_info.pVertexInputState = &vertex_input_info;
		pipeline_info.pInputAssemblyState = &input_assembly_info;
		pipeline_info.pViewportState = &viewport_state_info;
		pipeline_info.pRasterizationState = &rasterizer;
		pipeline_info.pMultisampleState = &multisample_state_info;
		pipeline_info.pDepthStencilState = nullptr;
		pipeline_info.pColorBlendState = &color_blending_info;
		pipeline_info.pDynamicState = &dynamicStateInfo;
		pipeline_info.layout = m_PipelineLayout;
		if (m_Renderer) {
			pipeline_info.renderPass = m_Renderer->GetRenderPass();

		}
		else {
			pipeline_info.renderPass = *m_Device.GetSwapchain().GetRenderPass();
		}
		pipeline_info.subpass = 0;
		pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
		pipeline_info.basePipelineIndex = -1;

		err = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, m_Allocator, &m_GraphicsPipeline);
		check_vk_result(err);

		vkDestroyShaderModule(device, vert_shader_module, m_Allocator);
		vkDestroyShaderModule(device, frag_shader_module, m_Allocator);
	}
	uint32_t* Pipeline::ReadShaderCode(const char* filename, uint32_t& codeSize) {
		std::ifstream inFile{ filename, std::ios::ate | std::ios::binary };

		if (!inFile.is_open()) {
			throw std::runtime_error("Failed to open the shader code file");
		}

		codeSize = (size_t)inFile.tellg();
		uint32_t* code = (uint32_t*)malloc(codeSize * sizeof(uint32_t));
		inFile.seekg(0);
		inFile.read(reinterpret_cast<char*>(code), codeSize);

		std::cout << codeSize << '\n';

		inFile.close();
		return code;
	}
	VkShaderModule Pipeline::CreateShaderModule(Device& device, uint32_t* code, uint32_t codeSize) {
		VkShaderModuleCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = codeSize;
		create_info.pCode = code;
		VkShaderModule shaderModule;
		VkResult err = vkCreateShaderModule(device.GetDevice(), &create_info, device.GetAllocator(),
			&shaderModule);
		check_vk_result(err);

		free(code);
		return shaderModule;
	}
}