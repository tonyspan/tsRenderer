#include "Window.h"

#include "Base.h"

#include "Log.h"

#include "Event.h"

#include <imgui_impl_glfw.h>

#include <GLFW/glfw3.h>

static bool s_Initialized = false;

Window::Window(const WindowDescription& desc)
{
	m_Data.Description = desc;

	if (!s_Initialized)
	{
		int success = glfwInit();
		ASSERT(GLFW_TRUE == success, "Couldn't init GLFW");
		s_Initialized = true;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_Window = glfwCreateWindow(desc.Width, desc.Height, desc.Title.data(), nullptr, nullptr);
	ASSERT(m_Window);

	glfwSetWindowUserPointer(m_Window, &m_Data);

	SetupCallbacks();
}

Window::~Window()
{
	glfwDestroyWindow(m_Window);
	m_Window = nullptr;

	glfwTerminate();
}

Scope<Window> Window::Create(const WindowDescription& props)
{
	return CreateScope<Window>(props);
}

void Window::OnUpdate()
{
	glfwPollEvents();
}

const std::string_view Window::GetTitle() const
{
	return m_Data.Description.Title;
}

void Window::SetWidth(uint32_t width)
{
	m_Data.Description.Width = width;
}

uint32_t Window::GetWidth() const
{
	return m_Data.Description.Width;
}

void Window::SetHeight(uint32_t height)
{
	m_Data.Description.Height = height;
}

uint32_t Window::GetHeight() const
{
	return m_Data.Description.Height;
}

std::pair<uint32_t, uint32_t> Window::GetSize() const
{
	return { m_Data.Description.Width, m_Data.Description.Height };
}

void Window::OnResize(uint32_t width, uint32_t height)
{
	if (m_Data.Description.Width == width && m_Data.Description.Height == height)
		return;

	SetWidth(width);
	SetHeight(width);
}

void Window::EnableVSync(bool enable)
{
	m_Data.Description.VSync = enable;
}

bool Window::HasVSync() const
{
	return m_Data.Description.VSync;
}

void Window::SetEventCallback(const std::function<void(Event&)>& cb)
{
	m_Data.EventCallback = cb;
}

void Window::SetupCallbacks()
{
	glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *static_cast<WindowData*>((glfwGetWindowUserPointer(window)));
			WindowDescription& desc = data.Description;

			int w = 0, h = 0;
			glfwGetFramebufferSize(window, &w, &h);

			ASSERT(width == w);
			ASSERT(height == h);

			desc.Width = static_cast<uint32_t>(width);
			desc.Height = static_cast<uint32_t>(height);

			ResizeEvent event;

			event.Width = desc.Width;
			event.Height = desc.Height;

			data.EventCallback(event);
		});

	glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *static_cast<WindowData*>((glfwGetWindowUserPointer(window)));

			QuitEvent event;

			data.EventCallback(event);
		});

	glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData& data = *static_cast<WindowData*>((glfwGetWindowUserPointer(window)));

			KeyPressedEvent event;

			switch (action)
			{
			case GLFW_PRESS:
			{
				event.Keycode = (KeyCode)key;
				event.RepeatCount = 0;

				break;
			}
			case GLFW_RELEASE:
			{
				break;
			}
			case GLFW_REPEAT:
			{
				event.Keycode = (KeyCode)key;
				event.RepeatCount = 1;

				break;
			}
			}

			data.EventCallback(event);
		});

	glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData& data = *static_cast<WindowData*>((glfwGetWindowUserPointer(window)));

			MouseButtonPressedEvent event;

			switch (action)
			{
			case GLFW_PRESS:
			case GLFW_RELEASE:
			{
				event.Button = (MouseButton)button;

				break;
			}
			}

			data.EventCallback(event);
		});

	glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowData& data = *static_cast<WindowData*>((glfwGetWindowUserPointer(window)));

			MouseMotionEvent event;

			event.Position = { static_cast<float>(xOffset), static_cast<float>(yOffset) };

			data.EventCallback(event);
		});

}

template<>
GLFWwindow* Window::GetHandle<GLFWwindow>() const
{
	return m_Window;
}