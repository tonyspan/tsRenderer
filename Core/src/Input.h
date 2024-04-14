#pragma once

#include <glm/glm.hpp>

enum class KeyCode { NONE, A, D, S, W };
enum class MouseButton { LMB, MMB, RMB };

class Window;

class Input
{
public:
	static void SetWindow(Window& window);
	static bool IsKeyPressed(KeyCode keyCode);
	static bool IsMousePressed(MouseButton mouseButton);
	static glm::vec2 MousePosition();

	static void HideCursor(bool hide);
};