#pragma once

#include "Base.h"

class CommandBuffer;
class Window;

class Gui
{
public:
	static Ref<Gui> Create(const Window& window);

	Gui(const Window& window);
	~Gui() = default;

	void Init(const Window& window);
	void Shutdown();

	void NewFrame();
	void Render(CommandBuffer& commandBuffer);
};