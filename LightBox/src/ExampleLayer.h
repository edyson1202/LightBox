#pragma once

#include <iostream>

#include "Layer.h"
#include "Device.h"
#include "Camera.h"
#include "Renderer2.h"
#include "Timer.h"
#include "Renderer.h"


namespace LightBox {
	class ExampleLayer : public LightBox::Layer
	{
	public:
		ExampleLayer(Device& device);
		virtual void OnUpdate(float ts) override;
		virtual void OnUIRender() override;
		void Render();
		Renderer2& GetRenderer() { return *m_Renderer2; }

	private:
		Device& m_Device;

		Renderer2* m_Renderer2;
		Camera* m_Camera;
		Renderer m_Renderer;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		float m_LastRenderTime = 0.0f;
	};
}
