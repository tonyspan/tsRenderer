#include "Instance.h"

#include "Window.h"

#include "Log.h"

#define VOLK_IMPLEMENTATION
#include <volk.h>
#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

std::vector<const char*> g_ValidationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

static std::vector<const char*> GetRequiredExtensions()
{
	uint32_t count = 0;

	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
	ASSERT(glfwExtensions || 0 != count);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + count);

	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

static bool IsValidationLayersSupported()
{
	uint32_t count;
	vkEnumerateInstanceLayerProperties(&count, nullptr);

	std::vector<VkLayerProperties> availableLayers(count);
	vkEnumerateInstanceLayerProperties(&count, availableLayers.data());

	for (const char* layerName : g_ValidationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
			return false;
	}

	return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
	{
		LOG("[TRACE]: Validation layer: %s", pCallbackData->pMessage);
		break;
	}
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
	{
		LOG("[INFO]: Validation layer: %s", pCallbackData->pMessage);
		break;
	}
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
	{
		LOG("[WARN]: Validation layer: %s", pCallbackData->pMessage);
		break;
	}
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
	{
		LOG("[ERROR]: Validation layer: %s", pCallbackData->pMessage);
		break;
	}
	default:
		break;
	}

	return VK_FALSE;
}

static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo)
{
	ZeroInitVkStruct(debugCreateInfo, VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT);

	debugCreateInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	debugCreateInfo.pfnUserCallback = DebugCallback;
	debugCreateInfo.pUserData = nullptr;
}

Instance::Instance(const Window& window)
{
	CreateInstance(window);
	SetupDebugMessenger();
}

Instance::~Instance()
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Handle::GetHandle(), "vkDestroyDebugUtilsMessengerEXT");
	func(Handle::GetHandle(), m_DebugUtilsMessenger, nullptr);

	vkDestroyInstance(Handle::GetHandle(), nullptr);

	volkFinalize();
}

void Instance::CreateInstance(const Window& window)
{
	ASSERT(glfwVulkanSupported() == GLFW_TRUE);

	VkResult result = volkInitialize();
	ASSERT(VK_SUCCESS == result);

	VkApplicationInfo appInfo;
	ZeroInitVkStruct(appInfo, VK_STRUCTURE_TYPE_APPLICATION_INFO);

	appInfo.pApplicationName = window.GetTitle().data();
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo createInfo;
	ZeroInitVkStruct(createInfo, VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);

	createInfo.pApplicationInfo = &appInfo;

	const auto& extensions = GetRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledLayerCount = 0;

	ASSERT(IsValidationLayersSupported(), "Validation layers requested aren't available");

	createInfo.enabledLayerCount = static_cast<uint32_t>(g_ValidationLayers.size());
	createInfo.ppEnabledLayerNames = g_ValidationLayers.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	PopulateDebugMessengerCreateInfo(debugCreateInfo);
	createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

	auto& handle = Handle::GetHandle();

	result = vkCreateInstance(&createInfo, nullptr, &handle);
	ASSERT(handle, "Instance creation failed");

	volkLoadInstance(handle);
}

void Instance::SetupDebugMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Handle::GetHandle(), "vkCreateDebugUtilsMessengerEXT");
	auto res = func(Handle::GetHandle(), &createInfo, nullptr, &m_DebugUtilsMessenger);
	ASSERT(m_DebugUtilsMessenger, "DebugUtilsMessenger creation failed");
}
