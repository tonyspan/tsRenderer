#include "Window.h"

#include "Base.h"

#include "Log.h"

#include <imgui_impl_sdl2.h>

#include <SDL.h>

static bool s_SDLInitialized = false;

Window::EventHandler::EventHandler()
	: m_Event(new SDL_Event())
{
}

Window::EventHandler::~EventHandler()
{
	delete m_Event;
	m_Event = nullptr;
}

void Window::EventHandler::Register(EventType type, EventCallbackFn&& cb)
{
	if (m_Callbacks.contains(type))
	{
		LOG("Event already registered");
		return;
	}

	m_Callbacks[type] = cb;
}

void Window::EventHandler::Poll()
{
	while (SDL_PollEvent(m_Event))
	{
		ImGui_ImplSDL2_ProcessEvent(m_Event);

		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse || io.WantCaptureKeyboard)
			return;

		Event event;
		EventType eventType = EventType::UNDEFINED;
		switch (m_Event->type)
		{
		case SDL_QUIT:
		{
			event = { .Quit = true };
			eventType = EventType::QUIT;

			break;
		}
		case SDL_WINDOWEVENT:
		{
			const SDL_WindowEvent& windowEvent = m_Event->window;
			const uint8_t windowEventID = static_cast<uint8_t>(windowEvent.event); // cast isn't needed since SDL's Uint8 is uint8_t

			if (windowEventID == SDL_WINDOWEVENT_RESIZED)
			{
				event = { .Width = static_cast<uint32_t>(windowEvent.data1), .Height = static_cast<uint32_t>(windowEvent.data2) };
				eventType = EventType::RESIZED;
			}
			else if (windowEventID == SDL_WINDOWEVENT_MINIMIZED)
			{
				event = { .IsMinimized = true };
				eventType = EventType::MININIZED;

				// To reduce CPU usage while minimized
				// Doesn't work as expected
				//while (SDL_WaitEvent(m_Event))
				//{
				//	if (windowEventID == SDL_WINDOWEVENT_RESTORED)
				//	{
				//		//event = { .IsMinimized = false };
				//		//eventType = EventType::RESTORED;
				//		break;
				//	}
				//}
			}
			else if (windowEventID == SDL_WINDOWEVENT_RESTORED)
			{
				event = { .IsMinimized = false };
				eventType = EventType::RESTORED;
			}

			break;
		}
		default:
			break;
		}

		if (m_Callbacks.contains(eventType))
			m_Callbacks.at(eventType)(event);
	}
}

Window::Window(const WindowDescription& desc)
	: m_Description(desc)
{
	if (!s_SDLInitialized)
	{
		int success = SDL_Init(SDL_INIT_VIDEO);
		ASSERT(!success, "Couldn't init SDL!");
		s_SDLInitialized = true;
	}

	auto flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

	if (desc.Maximized)
		flags = (SDL_WindowFlags)(flags | SDL_WINDOW_MAXIMIZED);

	m_Window = SDL_CreateWindow(desc.Title.data(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, desc.Width, desc.Height, flags | SDL_WINDOW_VULKAN);
	ASSERT(m_Window, "Unable to create a window. SDL error: %s", SDL_GetError());
}

Window::~Window()
{
	SDL_DestroyWindow(m_Window);
	m_Window = nullptr;

	SDL_Quit();
}

Scope<Window> Window::Create(const WindowDescription& props)
{
	return CreateScope<Window>(props);
}

void Window::OnUpdate()
{
	m_EventHandler.Poll();
}

const std::string_view Window::GetTitle() const
{
	return m_Description.Title;
}

void Window::SetWidth(uint32_t width)
{
	m_Description.Width = width;
}

uint32_t Window::GetWidth() const
{
	return m_Description.Width;
}

void Window::SetHeight(uint32_t height)
{
	m_Description.Height = height;
}

uint32_t Window::GetHeight() const
{
	return m_Description.Height;
}

void Window::OnResize(uint32_t width, uint32_t height)
{
	if (m_Description.Width == width && m_Description.Height == height)
		return;

	SetWidth(width);
	SetHeight(width);
}

void Window::EnableVSync(bool enable)
{
	m_Description.VSync = enable;
}

bool Window::HasVSync() const
{
	return m_Description.VSync;
}

void Window::RegisterCallback(EventType type, Window::EventHandler::EventCallbackFn&& cb)
{
	m_EventHandler.Register(type, std::move(cb));
}

template<>
SDL_Window* Window::GetHandle<SDL_Window>() const
{
	return m_Window;
}