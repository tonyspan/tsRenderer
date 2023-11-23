#pragma once

#include "Base.h"

#include <string_view>
#include <functional>
#include <unordered_map>

struct SDL_Window;
union SDL_Event;

struct WindowDescription
{
	std::string_view Title;
	uint32_t Width;
	uint32_t Height;
	std::string_view Icon;
	bool VSync = false;
	bool Maximized = false;

	WindowDescription(const std::string_view title = "VKRenderer", uint32_t width = 1600, uint32_t height = 900)
		: Title(title), Width(width), Height(height)
	{
	}
};

// TODO: Move EventType, Event elsewhere?

enum class EventType : int { UNDEFINED = 0, QUIT, RESIZED, MININIZED, RESTORED };

struct Event
{
	uint32_t Width = 0;
	uint32_t Height = 0;
	bool Quit = false;
	bool IsMinimized = false;
};

class Window
{
	class EventHandler
	{
	public:
		EventHandler();
		~EventHandler();

		using EventCallbackFn = std::function<void(Event&)>;

		void Register(EventType type, EventCallbackFn&& cb);

		void Poll();

	private:
		std::unordered_map<EventType, EventCallbackFn> m_Callbacks;
		SDL_Event* m_Event = nullptr;
	};

public:
	Window(const WindowDescription& desc);
	~Window();

	static Scope<Window> Create(const WindowDescription& desc);

	void OnUpdate();

	const std::string_view GetTitle() const;

	[[deprecated("Use OnResize()")]]
	void SetWidth(uint32_t width);
	uint32_t GetWidth() const;

	[[deprecated("Use OnResize()")]]
	void SetHeight(uint32_t height);
	uint32_t GetHeight() const;

	void OnResize(uint32_t width, uint32_t height);

	void EnableVSync(bool enable);
	bool HasVSync() const;

	void RegisterCallback(EventType type, Window::EventHandler::EventCallbackFn&& cb);

	template<typename T>
	T* GetHandle() const;
private:
	SDL_Window* m_Window = nullptr;
	WindowDescription m_Description;

	EventHandler m_EventHandler;
};
