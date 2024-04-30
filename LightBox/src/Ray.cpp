#include "Ray.h"

namespace LightBox {
	Vector3 Ray::GetRayAt(float t) const {
		return m_Origin + t * m_Direction;
	}
}