#pragma once

#include "KeyCodes.h"
#include "Math/Vector2.h"

#include <glm/glm.hpp>

namespace LightBox {

	class Input
	{
	public:
		static bool IsKeyDown(KeyCode keycode);
		static bool IsMouseButtonDown(MouseButton button);

		static glm::vec2 GetMousePosition();
		static Vector2 GetMousePosition2();

		static void SetCursorMode(CursorMode mode);
	};

}
