#pragma once

namespace LightBox {
	class Layer
	{
	public:
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDettach() {}

		virtual void OnUpdate(float ts) {}
		virtual void OnUIRender(uint32_t current_frame) {}

		virtual void Render() {}
		virtual void RecordCommands(VkCommandBuffer& command_buffer, uint32_t imageIndex) {}
	};
}