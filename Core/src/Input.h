#pragma once

#include <glm/glm.hpp>

enum class KeyCode { A = 4, D = 7, S = 22, W = 26 };
enum class MouseButton { LMB = 1, MMB = 2, RMB = 3 };

class Input
{
public:
	static bool IsKeyPressed(KeyCode keyCode);
	static bool IsMousePressed(MouseButton mouseButton);
	static glm::vec2 MousePosition();

	static void HideCursor(bool hide);
};