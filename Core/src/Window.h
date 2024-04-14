#pragma once

#include "Base.h"

#include <string_view>
#include <functional>
#include <utility>

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

struct GLFWwindow;
struct Event;

class Window
{
public:
	Window(const WindowDescription& desc);
	~Window();

	static Scope<Window> Create(const WindowDescription& desc);

	void OnUpdate();

	const std::string_view GetTitle() const;

	[[deprecated("Use OnResize()")]]
	void SetWidth(uint32_t width);
	[[deprecated("Use GetSize()")]]
	uint32_t GetWidth() const;

	[[deprecated("Use OnResize()")]]
	void SetHeight(uint32_t height);
	[[deprecated("Use GetSize()")]]
	uint32_t GetHeight() const;

	std::pair<uint32_t, uint32_t> GetSize() const;
	void OnResize(uint32_t width, uint32_t height);

	void EnableVSync(bool enable);
	bool HasVSync() const;

	void SetEventCallback(const std::function<void(Event&)>& cb);

	template<typename T>
	T* GetHandle() const;
private:
	void SetupCallbacks();
private:
	struct WindowData
	{
		WindowDescription Description;
		std::function<void(Event&)> EventCallback;
	} m_Data;

	GLFWwindow* m_Window = nullptr;
};
