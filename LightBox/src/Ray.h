#pragma once

#include "Math/Vector3.h"

namespace LightBox 
{
	class Ray
	{
	public:
		Ray() = default;
		Ray(const Vector3& origin, const Vector3& direction)
			: m_Origin(origin), m_Direction(direction) {}

		Vector3 GetOrigin() const { return m_Origin; }
		Vector3 GetDirection() const { return m_Direction; }

		Vector3 GetRayAt(float t) const;
	public:
		Vector3 m_Origin;
		Vector3 m_Direction;
	};
}