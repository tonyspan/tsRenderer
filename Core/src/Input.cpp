#include "Input.h"

#include "Log.h"

#include "Window.h"

#include <GLFW/glfw3.h>

static Window* s_Window = nullptr;

static int Convert(KeyCode keyCode)
{
	switch (keyCode)
	{
	case KeyCode::NONE:
		return GLFW_KEY_MENU;
	case KeyCode::A:
		return GLFW_KEY_A;
	case KeyCode::D:
		return GLFW_KEY_D;
	case KeyCode::S:
		return GLFW_KEY_S;
	case KeyCode::W:
		return GLFW_KEY_W;
	default:
		break;
	}

	return 0;
}

static int Convert(MouseButton button)
{
	switch (button)
	{
	case MouseButton::LMB:
		return GLFW_MOUSE_BUTTON_LEFT;
	case MouseButton::MMB:
		return GLFW_MOUSE_BUTTON_MIDDLE;
	case MouseButton::RMB:
		return GLFW_MOUSE_BUTTON_RIGHT;
	default:
		break;
	}

	return 0;
}

void Input::SetWindow(Window& window)
{
	s_Window = &window;
}

bool Input::IsKeyPressed(KeyCode keyCode)
{
	ASSERT(s_Window);

	auto state = glfwGetKey(s_Window->GetHandle<GLFWwindow>(), Convert(keyCode));
	return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::IsMousePressed(MouseButton mouseButton)
{
	ASSERT(s_Window);

	auto state = glfwGetMouseButton(s_Window->GetHandle<GLFWwindow>(), Convert(mouseButton));
	return state == GLFW_PRESS;
}

glm::vec2 Input::MousePosition()
{
	ASSERT(s_Window);

	double x, y;
	glfwGetCursorPos(s_Window->GetHandle<GLFWwindow>(), &x, &y);

	return { static_cast<float>(x), static_cast<float>(y) };
}

void Input::HideCursor(bool hide)
{
	ASSERT(s_Window);

	glfwSetInputMode(s_Window->GetHandle<GLFWwindow>(), GLFW_CURSOR, hide ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
}
