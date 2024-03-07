#include "Gui.h"

#include "Context.h"
#include "Instance.h"
#include "Device.h"
#include "Swapchain.h"
#include "CommandBuffer.h"
#include "Texture.h"
#include "RenderPass.h"

#include "Window.h"

#include "Log.h"

#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_vulkan.h>

#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

static VkDescriptorPool s_DescriptorPool = VK_NULL_HANDLE;

static VkDescriptorPool CreateDescriptorPool(VkDevice device)
{
	VkDescriptorPool pool = VK_NULL_HANDLE;

	const VkDescriptorPoolSize poolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER,                1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000 }
	};

	VkDescriptorPoolCreateInfo poolInfo = {};
	ZeroInitVkStruct(poolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);

	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
	poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
	poolInfo.pPoolSizes = poolSizes;

	VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool);
	VK_CHECK_RESULT(result);
	ASSERT(pool, "ImGui Descriptor pool creation failed");

	return pool;
}

Ref<Gui> Gui::Create(const Window& window)
{
	return CreateRef<Gui>(window);
}

Gui::Gui(const Window& window)
{
	Init(window);
}

void Gui::Init(const Window& window)
{
	const auto& device = Context::GetDevice();
	const auto& physicalDevice = device.GetPhysicalDevice();
	const auto& swapchain = Context::GetSwapchain();

	s_DescriptorPool = CreateDescriptorPool(device.GetHandle());

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

	ImGui_ImplSDL2_InitForVulkan(window.GetHandle<SDL_Window>());

	ImGui_ImplVulkan_InitInfo initInfo = {};

	initInfo.Instance = Context::GetInstance().GetHandle();
	initInfo.PhysicalDevice = physicalDevice.GetHandle();
	initInfo.Device = device.GetHandle();
	initInfo.QueueFamily = physicalDevice.GetQueueFamilyIndices().GraphicsIndex.value();
	initInfo.Queue = device.GetGraphicsQueue();
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.DescriptorPool = s_DescriptorPool;
	initInfo.Subpass = 0;
	initInfo.MinImageCount = 2;
	initInfo.ImageCount = swapchain.GetImageCount();
	initInfo.MSAASamples = physicalDevice.GetMsaaSamples();
	initInfo.Allocator = nullptr;
	initInfo.CheckVkResultFn = [](VkResult result) { VK_CHECK_RESULT(result); };

	ImGui_ImplVulkan_Init(&initInfo, swapchain.GetRenderPass()->GetHandle());

	// Upload Fonts
	{
		Ref<CommandBuffer> commandBuffer = CommandBuffer::Create(true);
		commandBuffer->BeginSingleTime();

		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer->GetHandle());

		commandBuffer->EndSingleTime();

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
}

void Gui::Shutdown()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	vkDestroyDescriptorPool(Context::GetDevice().GetHandle(), s_DescriptorPool, nullptr);
}

void Gui::Render(CommandBuffer& commandBuffer)
{
	ImGui::Render();
	ImDrawData* drawData = ImGui::GetDrawData();
	ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer.GetHandle());
}

void Gui::NewFrame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}
