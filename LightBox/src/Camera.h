#pragma once

#include <vector>

#include "Vector3.h"
#include "Mat4.h"
#include "Vector2.h"

namespace LightBox {
	class Vector3;
	class Camera {
	public:
		Camera(float fov, float near_plane, float far_plane);

		bool OnUpdate(float ts);
		void OnResize(uint32_t width, uint32_t height);

		const Mat4& GetProjection() const { return m_Projection; }
		const Mat4& GetView() const { return m_View; }
		float GetFOV() { return m_FOV; }

		const Vector3& GetPosition() { return m_Position; }
		const std::vector<Vector3>& GetRayDirections() { return m_RayDirections; }

		void SetFOV(float fov);
	private:
		void RecalculateProjection();
		void RecalculateView();

		void RecalculateRayDirections();
	private:
		Vector3 m_Position;
		Vector3 m_ForwardDirection;
		float m_Speed = 5.f;
		float m_FOV;
		float m_NearPlane;
		float m_FarPlane;

		Vector2 m_LastMousePos{ 0.f, 0.f };

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		Mat4 m_Projection;
		Mat4 m_InverseProjection;
		Mat4 m_CameraToWorld;
		Mat4 m_View;

		std::vector<Vector3> m_RayDirections;
	};
}
