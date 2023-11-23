#include "Input.h"

#include <SDL.h>

bool Input::IsKeyPressed(KeyCode keyCode)
{
	const Uint8* state = SDL_GetKeyboardState(NULL);
	return state[static_cast<SDL_Scancode>(keyCode)];
}

bool Input::IsMousePressed(MouseButton mouseButton)
{
	const auto state = SDL_GetMouseState(NULL, NULL);
	return state == SDL_BUTTON(static_cast<int>(mouseButton));
}

glm::vec2 Input::MousePosition()
{
	int x, y;
	SDL_GetMouseState(&x, &y);

	return { static_cast<float>(x), static_cast<float>(y) };
}

void Input::HideCursor(bool hide)
{
	SDL_ShowCursor(hide ? SDL_DISABLE : SDL_ENABLE);
}
