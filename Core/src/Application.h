#pragma once

#include "Window.h"

#include <utility>

struct Event;
struct ResizeEvent;

class CommandBuffer;

class IMGUI;

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
	virtual void OnRender(CommandBuffer& commandBuffer) = 0;
	virtual void OnShutdown() = 0;
	virtual void OnEvent(Event& event) = 0;
private:
	void AppInit();
	void AppShutdown();
	void AppEvent(Event& event);
	void OnResize(ResizeEvent& event);
private:
	Scope<Window> m_Window;
	bool m_ShouldClose = false;
	bool m_Minimized = false;

	Ref<IMGUI> m_ImGui;
};