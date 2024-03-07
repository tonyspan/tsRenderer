#include "Enums.h"

#include "Base.h"

#include "Log.h"

#include <vulkan/vulkan.h>

VkFormat Convert(Format format)
{
	switch (format)
	{
	case Format::RGBA_8_SRGB:
		return VK_FORMAT_R8G8B8A8_SRGB;
	case Format::BGRA_8_SRGB:
		return VK_FORMAT_B8G8R8A8_SRGB;
	case Format::R8_UINT:
		return VK_FORMAT_R8_UINT;
	case Format::R32_UINT:
		return VK_FORMAT_R32_UINT;
	case Format::R32_SFLOAT:
		return VK_FORMAT_R32_SFLOAT;
	case Format::RGBA_32_SFLOAT:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case Format::RGB_32_SFLOAT:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case Format::RG_32_SFLOAT:
		return VK_FORMAT_R32G32_SFLOAT;
	case Format::D32_SFLOAT:
		return VK_FORMAT_D32_SFLOAT;
	default:
		break;
	}

	ASSERT(false, "Unknown format type");
	return VK_FORMAT_UNDEFINED;
}

uint32_t FormatBytesPerPixel(Format format)
{
	switch (format)
	{
	case Format::RGBA_8_SRGB:
		return 4;
	default:
		break;
	}

	ASSERT(false, "Unknown format type");
	return 0;
}

static bool CheckMSAASamples(uint32_t msaaNumSamples)
{
	if ((msaaNumSamples == 1) || (msaaNumSamples == 2) || (msaaNumSamples == 4) || (msaaNumSamples == 8))
		return true;

	return false;
}

VkSampleCountFlagBits Convert(uint32_t msaaNumSamples)
{
	ASSERT(CheckMSAASamples(msaaNumSamples), "Invalid MSAA sample count");

#define MSAA_NUM_SAMPLE_CASE(msaaNumSample) \
	case msaaNumSample: return VK_SAMPLE_COUNT_##msaaNumSample##_BIT

	switch (msaaNumSamples)
	{
		MSAA_NUM_SAMPLE_CASE(1);
		MSAA_NUM_SAMPLE_CASE(2);
		MSAA_NUM_SAMPLE_CASE(4);
		MSAA_NUM_SAMPLE_CASE(8);
	default:
		break;
	}

	ASSERT(false, "Unknown MSAA sample count");
	return VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
}

VkShaderStageFlagBits Convert(StageFlag stage)
{
	switch (stage)
	{
	case StageFlag::VERTEX:
		return VK_SHADER_STAGE_VERTEX_BIT;
	case StageFlag::FRAGMENT:
		return VK_SHADER_STAGE_FRAGMENT_BIT;
	case StageFlag::ALL_GRAPHICS:
		return VK_SHADER_STAGE_ALL_GRAPHICS;
	default:
		break;
	}

	ASSERT(false, "Unknown shader stage");
	return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
}

const char* ShaderStageString(StageFlag stage)
{
	switch (stage)
	{
	case StageFlag::VERTEX:
		return "Vertex";
	case StageFlag::FRAGMENT:
		return "Fragment";
	default:
		break;
	}

	ASSERT(false, "Unknown shader stage");
	return nullptr;
}

VkDescriptorType Convert(DescriptorType type)
{
	switch (type)
	{
	case DescriptorType::UNIFORM_BUFFER:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case DescriptorType::COMBINED_IMAGE_SAMPLER:
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	default:
		break;
	}

	ASSERT(false, "Unknown DescriptorType");
	return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

VkCullModeFlagBits Convert(CullMode mode)
{
	switch (mode)
	{
	case CullMode::NONE:
		return VK_CULL_MODE_NONE;
	case CullMode::FRONT:
		return VK_CULL_MODE_FRONT_BIT;
	case CullMode::BACK:
		return VK_CULL_MODE_BACK_BIT;
	case CullMode::FRONT_AND_BACK:
		return VK_CULL_MODE_FRONT_AND_BACK;
	default:
		break;
	}

	ASSERT(false, "Unknown Cull mode");
	return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
}

VkCompareOp Convert(CompareOp op)
{
	switch (op)
	{
	case CompareOp::NEVER:
		return VK_COMPARE_OP_NEVER;
	case CompareOp::LESS:
		return VK_COMPARE_OP_LESS;
	case CompareOp::EQUAL:
		return VK_COMPARE_OP_EQUAL;
	case CompareOp::LESS_OR_EQUAL:
		return VK_COMPARE_OP_LESS_OR_EQUAL;
	case CompareOp::GREATER:
		return VK_COMPARE_OP_GREATER;
	case CompareOp::NOT_EQUAL:
		return VK_COMPARE_OP_NOT_EQUAL;
	case CompareOp::GREATER_OR_EQUAL:
		return VK_COMPARE_OP_GREATER_OR_EQUAL;
	case CompareOp::ALWAYS:
		return VK_COMPARE_OP_ALWAYS;
	default:
		break;
	}

	ASSERT(false, "Unknown CompareOp");
	return VK_COMPARE_OP_MAX_ENUM;
}

VkPolygonMode Convert(PolygonMode mode)
{
	switch (mode)
	{
	case PolygonMode::FILL:
		return VK_POLYGON_MODE_FILL;
	case PolygonMode::LINE:
		return VK_POLYGON_MODE_LINE;
	case PolygonMode::POINT:
		return VK_POLYGON_MODE_POINT;
	case PolygonMode::FILL_RECTANGLE_NV:
		return VK_POLYGON_MODE_FILL_RECTANGLE_NV;
	default:
		break;
	}

	ASSERT(false, "Unknown PolygonMode");
	return VK_POLYGON_MODE_MAX_ENUM;
}

VkPrimitiveTopology Convert(PrimitiveTopology topology)
{
	switch (topology)
	{
	case PrimitiveTopology::POINT_LIST:
		return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	case PrimitiveTopology::LINE_LIST:
		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	case PrimitiveTopology::LINE_STRIP:
		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	case PrimitiveTopology::TRIANGLE_LIST:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	case PrimitiveTopology::TRIANGLE_STRIP:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	case PrimitiveTopology::TRIANGLE_FAN:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
	case PrimitiveTopology::LINE_LIST_WITH_ADJACENCY:
		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
	case PrimitiveTopology::LINE_STRIP_WITH_ADJACENCY:
		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
	case PrimitiveTopology::TRIANGLE_LIST_WITH_ADJACENCY:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
	case PrimitiveTopology::TRIANGLE_STRIP_WITH_ADJACENCY:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
	case PrimitiveTopology::PATCH_LIST:
		return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
	default:
		break;
	}

	ASSERT(false, "Unknown PrimitiveTopology");
	return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
}

VkFilter Convert(Filter filter)
{
	switch (filter)
	{
	case Filter::NEAREST:
		return VK_FILTER_NEAREST;
	case Filter::LINEAR:
		return VK_FILTER_LINEAR;
	default:
		break;
	}

	ASSERT(false, "Unknown Filter");
	return VK_FILTER_MAX_ENUM;
}
