#pragma once

#include <cstdint>

#include <tuple>
#include <concepts>

template<typename VkStruct, typename VkStructType>
	requires requires(VkStruct vkStruct)
{
	{ vkStruct.sType };
}
void ZeroInitVkStruct(VkStruct& vkStruct, const VkStructType& type)
{
	vkStruct = {};

	vkStruct.sType = type;
}

template <typename... Types>
//requires (std::is_pointer_v<Types> && ...)
class Handle
{
public:
	static_assert((std::is_pointer_v<Types> && ...), "Types must be pointers");

	Handle()
		: m_Handles(Init<Types...>())
	{
	}

	virtual ~Handle()
	{
		std::apply([]<typename... T>(T&...handles)
		{
			((handles = nullptr), ...);
		}, m_Handles);
	}

	template <typename T>
	T& GetHandle()
	{
		static_assert(HasType<T>());

		return std::get<T>(m_Handles);
	}

	template <typename T>
	const T& GetHandle() const
	{
		static_assert(HasType<T>());

		return std::get<T>(m_Handles);
	}

	template <typename T = std::tuple<Types...>, typename = std::enable_if_t<(std::tuple_size<T>::value == 1)>>
	decltype(auto) GetHandle()
	{
		return std::get<0>(m_Handles);
	}

	template <typename T = std::tuple<Types...>>
		requires (std::tuple_size<T>::value == 1)
	decltype(auto) GetHandle() const
	{
		return std::get<0>(m_Handles);
	}
private:
	template<typename T, typename... Rest>
	static constexpr auto Init()
	{
		if constexpr (sizeof...(Rest) == 0)
		{
			return std::make_tuple(static_cast<T>(nullptr));
		}
		else
		{
			return std::tuple_cat(std::make_tuple(static_cast<T>(nullptr)), Init<Rest...>());
		}
	}

	template<typename T>
	static constexpr bool HasType()
	{
		return std::disjunction_v<std::is_same<T, Types>...>;
	}
private:
	std::tuple<Types...> m_Handles;
};

#define VK_FWD_DECL_HANDLE(X) typedef struct X##_T* X;
#define VK_FWD_DECL_STRUCT(X) typedef struct X X;
#define VK_FWD_DECL_ENUM(X) typedef enum X : int X;

// Max value of all(?) Vulkan enums
#define VK_MAX_VALUE_ENUM (~0U >> 1) /* = 0x7FFFFFFF or 2147483647 */

typedef uint64_t VkDeviceSize;
typedef uint32_t VkFlags;
typedef VkFlags VkMemoryPropertyFlags;
typedef VkFlags VkBufferUsageFlags;
typedef VkFlags VkImageUsageFlags;
typedef VkFlags VkImageAspectFlags;
typedef VkFlags VkFormatFeatureFlags;
typedef VkFlags VkCommandPoolCreateFlags;
typedef VkFlags VkFenceCreateFlags;
typedef VkFlags VkImageCreateFlags;
typedef VkFlags VkShaderStageFlags;

VK_FWD_DECL_HANDLE(VkInstance)
VK_FWD_DECL_HANDLE(VkDebugUtilsMessengerEXT)
VK_FWD_DECL_HANDLE(VkPhysicalDevice)
VK_FWD_DECL_HANDLE(VkDevice)
VK_FWD_DECL_HANDLE(VkQueue)
VK_FWD_DECL_HANDLE(VkSurfaceKHR)
VK_FWD_DECL_HANDLE(VkSwapchainKHR)
VK_FWD_DECL_HANDLE(VkFramebuffer)
VK_FWD_DECL_HANDLE(VkImage)
VK_FWD_DECL_HANDLE(VkImageView)
VK_FWD_DECL_HANDLE(VkSampler)
VK_FWD_DECL_HANDLE(VkShaderModule)
VK_FWD_DECL_HANDLE(VkRenderPass)
VK_FWD_DECL_HANDLE(VkPipeline)
VK_FWD_DECL_HANDLE(VkPipelineLayout)
VK_FWD_DECL_HANDLE(VkDescriptorSet)
VK_FWD_DECL_HANDLE(VkDescriptorSetLayout)
VK_FWD_DECL_HANDLE(VkCommandPool)
VK_FWD_DECL_HANDLE(VkBuffer)
VK_FWD_DECL_HANDLE(VkDeviceMemory)
VK_FWD_DECL_HANDLE(VkDescriptorPool)
VK_FWD_DECL_HANDLE(VkCommandBuffer)
VK_FWD_DECL_HANDLE(VkSemaphore)
VK_FWD_DECL_HANDLE(VkFence)

VK_FWD_DECL_STRUCT(VkSurfaceCapabilitiesKHR)
VK_FWD_DECL_STRUCT(VkPipelineShaderStageCreateInfo)
VK_FWD_DECL_STRUCT(VkVertexInputBindingDescription)
VK_FWD_DECL_STRUCT(VkVertexInputAttributeDescription)
VK_FWD_DECL_STRUCT(VkSurfaceFormatKHR)
VK_FWD_DECL_STRUCT(VkPhysicalDeviceMemoryProperties)
VK_FWD_DECL_STRUCT(VkPhysicalDeviceFeatures)
VK_FWD_DECL_STRUCT(VkPhysicalDeviceProperties)
VK_FWD_DECL_STRUCT(VkQueueFamilyProperties)
VK_FWD_DECL_STRUCT(VkDescriptorSetLayoutBinding)
VK_FWD_DECL_STRUCT(VkPushConstantRange)

VK_FWD_DECL_ENUM(VkResult)
VK_FWD_DECL_ENUM(VkPresentModeKHR)
VK_FWD_DECL_ENUM(VkFormat)
VK_FWD_DECL_ENUM(VkImageTiling)
VK_FWD_DECL_ENUM(VkImageLayout)
VK_FWD_DECL_ENUM(VkImageType)
VK_FWD_DECL_ENUM(VkImageViewType)
VK_FWD_DECL_ENUM(VkSampleCountFlagBits)
VK_FWD_DECL_ENUM(VkShaderStageFlagBits)
VK_FWD_DECL_ENUM(VkCullModeFlagBits)
VK_FWD_DECL_ENUM(VkCompareOp)
VK_FWD_DECL_ENUM(VkPolygonMode)
VK_FWD_DECL_ENUM(VkPrimitiveTopology)
VK_FWD_DECL_ENUM(VkFilter)
VK_FWD_DECL_ENUM(VkDescriptorType)
VK_FWD_DECL_ENUM(VkCommandBufferLevel)

typedef union VkClearColorValue VkClearColorValue;

const char* VkResultString(VkResult result);

// Need to include "Log.h" before including this header
#define VK_CHECK_RESULT(X) { \
	VkResult _res = (X); \
	if (_res != VK_SUCCESS) \
	{ \
		SRC_LOC(); \
		LOG("%s, line %i: VkResult is %s", func, line, VkResultString(_res)); \
	} \
}