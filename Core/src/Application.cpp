#include "Application.h"

#include "Context.h"
#include "Device.h"
#include "DescriptorPool.h"
#include "Swapchain.h"
#include "CommandBuffer.h"
#include "Shader.h"

#include "Event.h"

#include "Input.h"

#include "Timer.h"
#include "Log.h"
#include "Profiler.h"

#include "IMGUII.h"
#include <imgui.h>

void Application::Run()
{
	AppInit();
	OnInit();

	Timer timer;

	auto& swapchain = Context::GetSwapchain();

	while (!m_ShouldClose)
	{
		m_Window->OnUpdate();

		float dt = timer.Elapsed();
		float dtMS = timer.ElapsedMS();
		timer.Reset();

		if (!m_Minimized)
		{
			OnUpdate(dt);

			swapchain.BeginFrame();
			m_ImGui->NewFrame();

			auto& commandBuffer = swapchain.GetCurrentCommandBuffer();

			commandBuffer.BeginRecording();
			commandBuffer.BeginRenderPass(*swapchain.GetRenderPass(), swapchain.GetCurrentFramebuffer());

			OnRender(commandBuffer);


			if (ImGui::Begin("Stats", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
			{
				ImGui::Text("FPS: %0.1f | Delta Time: %0.2f ms", 1.0f / dt, dtMS);

				for (const auto& [name, frameData] : PerFramePerfProfiler::GetPerFrameData())
					ImGui::Text("%s Time: %.2f ms", name.data(), frameData.Time);
			}
			ImGui::End();

			m_ImGui->Render(commandBuffer);

			commandBuffer.EndRenderPass();
			commandBuffer.EndRecording();

			swapchain.EndFrame();
		}
	}

	Context::GetDevice().WaitIdle();

	OnShutdown();
	AppShutdown();
}

std::pair<uint32_t, uint32_t> Application::GetSize() const
{
	const auto& desc = Context::GetSwapchain().GetDescription();
	const auto& [windowWidth, windowHeight] = m_Window->GetSize();

	ASSERT(desc.Width == windowWidth && desc.Height == windowHeight);

	return { desc.Width, desc.Height };
}

void Application::AppInit()
{
	WindowDescription desc;

	desc.VSync = true;
	m_Window = Window::Create(desc);

	m_Window->SetEventCallback(BIND_FUNC(AppEvent));

	Input::SetWindow(*m_Window);

	Context::Init(*m_Window);

	m_ImGui = IMGUI::Create(*m_Window);
}

void Application::AppShutdown()
{
	m_ImGui->Shutdown();

	Context::Shutdown();

	m_Window.reset();
}

void Application::AppEvent(Event& event)
{
	EventDispatcher dispatcher(event);

	dispatcher.Dispatch<QuitEvent>([&shouldClode = m_ShouldClose](QuitEvent& event)
		{
			shouldClode = true;
		});

	dispatcher.Dispatch<ResizeEvent>(BIND_FUNC(OnResize));

	OnEvent(event);
}

// For the current architecture works, but ...
// NOTE: Almost the whole app is destroyed and restarted to work which is far from optimal
// TODO: Fix somehow
void Application::OnResize(ResizeEvent& event)
{
	const uint32_t width = event.Width;
	const uint32_t height = event.Height;

	Context::GetDevice().WaitIdle();

	if (0 == width || 0 == height)
	{
		m_Minimized = true;
		return;
	}

	m_Minimized = false;

	OnShutdown();

	Context::GetSwapchain().OnResize(width, height);
	m_Window->OnResize(width, height);

	OnInit();

	Context::GetDevice().WaitIdle();
}
