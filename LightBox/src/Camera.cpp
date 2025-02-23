#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Input/Input.h"
#include "Math/Quat.h"

namespace LightBox {
	Mat4 LookAt(const Vector3& from, const Vector3& to, const Vector3& up) {
		Vector3 forward = (from - to).Normalize();
		Vector3 temp = up.Normalize();
		Vector3 right = Vector3::Cross(temp, forward);
		right = right.Normalize();

		Vector3 local_up = Vector3::Cross(forward, right);

		Mat4 mat;

		mat.m[0][0] = right.x, mat.m[1][0] = local_up.x, mat.m[2][0] = forward.x, mat.m[3][0] = from.x;
		mat.m[0][1] = right.y, mat.m[1][1] = local_up.y, mat.m[2][1] = forward.y, mat.m[3][1] = from.y;
		mat.m[0][2] = right.z, mat.m[1][2] = local_up.z, mat.m[2][2] = forward.z, mat.m[3][2] = from.z;

		mat.m[3][3] = 1;

		return mat;
	}
	Camera::Camera(float fov, float near_plane, float far_plane)
		: m_Fov(fov), m_NearPlane(near_plane), m_FarPlane(far_plane)
	{
		m_Position = Vector3(0, 0, 0);
		m_ForwardDirection = Vector3(0, 0, -1.f);
	}
	bool Camera::OnUpdate(float ts)
	{
		bool moved = false;

		Vector2 current_mouse_pos = Input::GetMousePosition2();
		Vector2 delta = (current_mouse_pos - m_LastMousePos) * 0.002f;
		m_LastMousePos = current_mouse_pos;

		if (!Input::IsMouseButtonDown(MouseButton::Right)) {
			Input::SetCursorMode(CursorMode::Normal);
			return false;
		}
		Input::SetCursorMode(CursorMode::Locked);

		Vector3 up_direction(0.f, 1.f, 0.f);
		Vector3 right_direction = Vector3::Cross(m_ForwardDirection, up_direction);

		// Movement
		if (Input::IsKeyDown(KeyCode::W)) {
			m_Position = m_Position + (m_Speed * ts * m_ForwardDirection); 
			m_Position = m_Position + m_ForwardDirection * m_Speed * ts;
			moved = true;
		}
		if (Input::IsKeyDown(KeyCode::S)) {
			m_Position = m_Position - (m_Speed * ts * m_ForwardDirection);
			moved = true;
		}
		if (Input::IsKeyDown(KeyCode::D)) {
			m_Position = m_Position + (m_Speed * ts * right_direction);
			moved = true;
		}
		if (Input::IsKeyDown(KeyCode::A)) {
			m_Position = m_Position - (m_Speed * ts * right_direction);
			moved = true;
		}
		if (Input::IsKeyDown(KeyCode::E)) {
			m_Position = m_Position + (m_Speed * ts * up_direction);
			moved = true;
		}if (Input::IsKeyDown(KeyCode::Q)) {
			m_Position = m_Position - (m_Speed * ts * up_direction);
			moved = true;
		}

		float rotation_speed = 0.8f;
		// Rotation
		if (delta.x != 0.f || delta.y != 0.f) {
;			float pitch_delta = delta.y * rotation_speed;
			float yaw_delta = delta.x * rotation_speed;

			Quat q = Quat::HamiltonProduct(Quat::AngleAxis(-pitch_delta, right_direction).Normalize(),
				Quat::AngleAxis(-yaw_delta, Vector3(0, 1, 0)).Normalize());
			m_ForwardDirection = Quat::Rotate(q, m_ForwardDirection);

			/*glm::vec3 right_dir;
			right_dir.x = right_direction.x;
			right_dir.y = right_direction.y;
			right_dir.z = right_direction.z;
			glm::vec3 forward_dir;
			forward_dir.x = m_ForwardDirection.x;
			forward_dir.y = m_ForwardDirection.y;
			forward_dir.z = m_ForwardDirection.z;
			glm::quat q_glm = glm::normalize(glm::cross(glm::angleAxis(-pitch_delta, right_dir),
				glm::angleAxis(-yaw_delta, glm::vec3(0.f, 1.0f, 0.0f))));
			forward_dir = glm::rotate(q_glm, forward_dir);*/

			/*m_ForwardDirection.x = forward_dir.x;
			m_ForwardDirection.y = forward_dir.y;
			m_ForwardDirection.z = forward_dir.z;*/
	
			moved = true;
		}

		if (moved) {
			RecalculateView();
			RecalculateRayDirections();
		}
		return moved;
	}
	void Camera::OnResize(const uint32_t width, const uint32_t height)
	{
		if (m_ViewportWidth == width && m_ViewportHeight == height)
			return;

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		std::cout << "Camera width: " << m_ViewportWidth << '\n';
		std::cout << "Camera height: " << m_ViewportHeight << '\n';

		RecalculateProjection();
		RecalculateRayDirections();
	}

	void Camera::Refresh()
	{
		RecalculateView();
		RecalculateProjection();
		RecalculateRayDirections();
	}

	void Camera::SetFov(float fov)
	{
		if (fov == m_Fov)
			return;

		m_Fov = fov;
		RecalculateProjection();
		RecalculateRayDirections();
	}
	void Camera::RecalculateProjection()
	{
		m_Projection = Mat4::GetProjectionMatrix(m_Fov, (float)m_ViewportHeight / (float)m_ViewportWidth, m_NearPlane, m_FarPlane);
		m_InverseProjection = m_Projection.GetInverse();
	}
	void Camera::RecalculateView()
	{
		m_CameraToWorld = LookAt(m_Position, m_Position + m_ForwardDirection, Vector3(0, 1, 0));
		m_View = m_CameraToWorld.GetInverse();
	}
	void Camera::RecalculateRayDirections()
	{
		m_RayDirections.resize(m_ViewportWidth * m_ViewportHeight);
		for (uint32_t y = 0; y < m_ViewportHeight; y++) {
			for (uint32_t x = 0; x < m_ViewportWidth; x++) {
				Vector2 norm_coord((float)x / (float)m_ViewportWidth, (float)y / (float)m_ViewportHeight);
				norm_coord.x = 2.f * norm_coord.x - 1.f; // [0, 1] -> [-1, 1]
				//norm_coord.y = 1.f - 2.f * norm_coord.y;
				norm_coord.y = 2.f * norm_coord.y - 1.f;

				Vector4 target = m_InverseProjection * Vector4(norm_coord.x, norm_coord.y, 1, 1);
				Vector3 ray_direction(m_CameraToWorld * Vector4((Vector3(target) / target.w).Normalize(), 0));

				m_RayDirections[x + y * m_ViewportWidth] = ray_direction;

				//m_RayDirections[x + y * m_ViewportWidth] = Vector3(m_CameraToWorld * Vector4((Vector3(target) / target.w).Normalize(), 0));
			}
		}
	}
}
