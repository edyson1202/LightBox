#pragma once

#include <vector>

#include "Math/Vector3.h"
#include "Math/Mat4.h"
#include "Math/Vector2.h"

namespace LightBox {
	class Vector3;
	class Camera {
	public:
		Camera(float fov, float near_plane, float far_plane);

		bool OnUpdate(float ts);
		void OnResize(const uint32_t width, const uint32_t height);

		// This is used when deserializing camera data, to ensure the camera is updated
		void Refresh();

		const Mat4& GetProjection() const { return m_Projection; }
		const Mat4& GetView() const { return m_View; }
		float GetFov() const { return m_Fov; }

		const Vector3& GetPosition() const { return m_Position; }
		const std::vector<Vector3>& GetRayDirections() { return m_RayDirections; }

		void SetFov(float fov);
	private:
		void RecalculateProjection();
		void RecalculateView();

		void RecalculateRayDirections();
	public:
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	private:
		Vector3 m_Position;
		Vector3 m_ForwardDirection;
		float m_Speed = 5.f;
		float m_Fov;
		float m_NearPlane;
		float m_FarPlane;

		Vector2 m_LastMousePos{ 0.f, 0.f };
		
		Mat4 m_Projection;
		Mat4 m_InverseProjection;
		Mat4 m_CameraToWorld;
		Mat4 m_View;

		std::vector<Vector3> m_RayDirections;

		friend class SceneSerializer;
	};
}
