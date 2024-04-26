#include "IMGUII.h"

#include "Context.h"
#include "Instance.h"
#include "Device.h"
#include "Swapchain.h"
#include "CommandBuffer.h"
#include "Texture.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "DescriptorPool.h"

#include "Window.h"

#include "Log.h"

#include <volk.h>
#include <vulkan/vulkan.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <GLFW/glfw3.h>

Ref<IMGUI> IMGUI::Create(const Window& window)
{
	return CreateRef<IMGUI>(window);
}

IMGUI::IMGUI(const Window& window)
{
	Init(window);
}

void IMGUI::Init(const Window& window)
{
	const auto& device = Context::GetDevice();
	const auto& physicalDevice = device.GetPhysicalDevice();
	const auto& swapchain = Context::GetSwapchain();

	m_DescriptorPool = CreateRef<DescriptorPool>(device);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

	VkInstance instance = Context::GetInstance().GetHandle();

	ImGui_ImplVulkan_LoadFunctions([](const char* funcName, void* vkInstance)
		{
			return vkGetInstanceProcAddr(volkGetLoadedInstance(), funcName);
		}, &instance);

	ImGui_ImplGlfw_InitForVulkan(window.GetHandle<GLFWwindow>(), true);

	ImGui_ImplVulkan_InitInfo initInfo = {};

	initInfo.Instance = instance;
	initInfo.PhysicalDevice = physicalDevice.GetHandle();
	initInfo.Device = device.GetHandle();
	initInfo.QueueFamily = physicalDevice.GetQueueFamilyIndices().GraphicsIndex.value();
	initInfo.Queue = device.GetGraphicsQueue();
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.DescriptorPool = m_DescriptorPool->GetHandle();
	initInfo.Subpass = 0;
	initInfo.MinImageCount = 2;
	initInfo.ImageCount = swapchain.GetImageCount();

	// TODO: Find a better way to deal with it
	const auto& msaaSamples = swapchain.GetRenderPass()->GetDescription().MSAAnumSamples;

	initInfo.MSAASamples = msaaSamples > 1 ? Convert(msaaSamples.value()) : VK_SAMPLE_COUNT_1_BIT;
	initInfo.Allocator = nullptr;
	initInfo.CheckVkResultFn = [](VkResult result) { VK_CHECK_RESULT(result); };

	ImGui_ImplVulkan_Init(&initInfo, swapchain.GetRenderPass()->GetHandle());

	// Upload Fonts
	{
		Ref<CommandBuffer> commandBuffer = CommandBuffer::Create(true);
		commandBuffer->BeginRecording(true);

		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer->GetHandle());

		commandBuffer->EndRecordingAndSubmit();

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
}

void IMGUI::Shutdown()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	m_DescriptorPool.reset();
}

void IMGUI::Render(CommandBuffer& commandBuffer)
{
	ImGui::Render();
	ImDrawData* drawData = ImGui::GetDrawData();
	ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer.GetHandle());
}

void IMGUI::NewFrame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}
