#pragma once

#include "Window.h"

#include <utility>

struct Event;

class CommandBuffer;

class Gui;

class Application
{
public:
	Application() = default;
	~Application() = default;

	void Run();

	std::pair<uint32_t, uint32_t> GetSize() const;
protected:
	virtual void OnInit() = 0;
	virtual void OnUpdate(float dt) = 0;
	virtual void Render(CommandBuffer& commandBuffer) = 0;
	virtual void OnShutdown() = 0;
	virtual void OnEvent(Event& event) = 0;
private:
	void OnClose(Event& event);
	void OnResize(Event& event);
	void OnMinimize(Event& event);
private:
	Scope<Window> m_Window;
	bool m_ShouldClose = false;
	bool m_Minimized = false;

	Ref<Gui> m_ImGui;
};