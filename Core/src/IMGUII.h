#pragma once

#include "Base.h"

class CommandBuffer;
class Window;

class IMGUI
{
public:
	static Ref<IMGUI> Create(const Window& window);

	IMGUI(const Window& window);
	~IMGUI() = default;

	void Init(const Window& window);
	void Shutdown();

	void NewFrame();
	void Render(CommandBuffer& commandBuffer);
};