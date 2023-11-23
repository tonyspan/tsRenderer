#include "VK.h"

#include "Log.h"

#include <vulkan/vk_enum_string_helper.h>

const char* VkResultString(VkResult result)
{
	return string_VkResult(result);
}
