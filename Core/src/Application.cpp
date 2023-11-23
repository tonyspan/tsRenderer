#include "Application.h"

#include "Context.h"
#include "Device.h"
#include "DescriptorPool.h"
#include "Swapchain.h"
#include "CommandBuffer.h"
#include "Shader.h"

#include "Timer.h"
#include "Log.h"
#include "Profiler.h"

#include "Gui.h"
#include <imgui.h>

void Application::Run()
{
	WindowDescription desc;

	desc.VSync = true;
	m_Window = Window::Create(desc);

	m_Window->RegisterCallback(EventType::QUIT, BIND_FUNC(OnClose));
	m_Window->RegisterCallback(EventType::RESIZED, BIND_FUNC(OnResize));
	m_Window->RegisterCallback(EventType::MININIZED, BIND_FUNC(OnMinimize));
	m_Window->RegisterCallback(EventType::RESTORED, BIND_FUNC(OnMinimize));

	Context::Init(*m_Window);

	m_ImGui = Gui::Create(*m_Window);

	auto& swapchain = Context::GetSwapchain();

	OnInit();

	Timer timer;

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
			Render(commandBuffer);

			if (ImGui::Begin("Stats", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
			{
				ImGui::Text("FPS: %0.1f | Delta Time: %0.2f ms", 1.0f / dt, dtMS);

				for (const auto& [name, timeData] : PerFramePerfProfiler::GetPerFrameData())
					ImGui::Text("%s Time: %.2f ms", name.data(), timeData.Time);
			}
			ImGui::End();

			m_ImGui->Render(commandBuffer);

			swapchain.EndFrame();
		}
	}

	Context::GetDevice().WaitIdle();

	OnShutdown();

	m_ImGui->Shutdown();

	Context::Shutdown();

	m_Window.reset();
}

std::pair<uint32_t, uint32_t> Application::GetSize() const
{
	const auto& desc = Context::GetSwapchain().GetDescription();
	return { desc.Width, desc.Height };
}

void Application::OnClose(Event& event)
{
	m_ShouldClose = true;
}

// For the current architecture works, but ...
// NOTE: Almost the whole app is destroyed and restarted to work which is far from optimal
// TODO: Fix somehow
void Application::OnResize(Event& event)
{
	uint32_t width = event.Width;
	uint32_t height = event.Height;

	Context::GetDevice().WaitIdle();

	OnShutdown();

	Context::GetDescriptorPool().Recreate();

	Context::GetSwapchain().OnResize(width, height);
	m_Window->OnResize(width, height);

	OnInit();

	//Context::GetDevice().WaitIdle();
}

void Application::OnMinimize(Event& event)
{
	m_Minimized = event.IsMinimized;
}
