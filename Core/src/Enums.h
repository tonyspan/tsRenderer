#pragma once

#include "VK.h"

enum class Format : int
{
	UNDEFINED = 0,

	// Color Attachments

	// 8 bits/component
	RGBA_8_SRGB,
	BGRA_8_SRGB,

	// Used in Vertex

	R8_UINT,
	R32_UINT,
	R32_SFLOAT,

	// 32 bits/component
	RGBA_32_SFLOAT,
	// 32 bits/component
	RGB_32_SFLOAT,
	// 32 bits/component
	RG_32_SFLOAT,

	// Depth

	D32_SFLOAT
};

VkFormat Convert(Format format);
uint32_t GetStrideFromFormat(VkFormat format);
uint32_t FormatBytesPerPixel(Format format);
Format FormatBytesPerPixel(uint32_t channels);

// Unused
enum class MSAASamples
{
};

VkSampleCountFlagBits Convert(uint8_t msaaNumSamples);

enum class StageFlag : int
{
	UNDEFINED = 0,

	VERTEX,
	FRAGMENT,
	ALL_GRAPHICS
};

VkShaderStageFlagBits Convert(StageFlag stage);
const char* ShaderStageString(StageFlag stage);

enum class DescriptorType : int
{
	UNDEFINED = 0,

	// Represents a buffer, and supports read-only access in a shader
	UNIFORM_BUFFER,
	// Includes both a sampled image and a sampler
	COMBINED_IMAGE_SAMPLER,

	// Unused
	// Represents an image view, and supports filtered (sampled) and unfiltered read-only acccess in a shader
	SAMPLED_IMAGE,

	// Unused
	// Contains state that controls how sampled image data is sampled (or filtered) when accessed in a shader
	// Also, describes a VkSampler object
	SAMPLER,

	// Unused
	// A uniform buffer whose offset is specified each time the uniform buffer is bound to 
	// a command buffer via a descriptor set
	DYNAMIC_UNIFORM_BUFFER,

	// Unused
	// Represents an image view, and supports unfiltered loads, stores, and atomics in a shader
	STORAGE_IMAGE
};

VkDescriptorType Convert(DescriptorType type);
DescriptorType Convert(VkDescriptorType type);
const char* DescriptorTypeString(DescriptorType type);

enum class CullMode
{
	NONE,
	FRONT,
	BACK,
	FRONT_AND_BACK
};

VkCullModeFlagBits Convert(CullMode mode);

enum class CompareOp
{
	NEVER,
	LESS,
	EQUAL,
	LESS_OR_EQUAL,
	GREATER,
	NOT_EQUAL,
	GREATER_OR_EQUAL,
	ALWAYS
};

VkCompareOp Convert(CompareOp op);

enum class PolygonMode
{
	UNDEFINED = 0,

	FILL,
	LINE,
	POINT,
	FILL_RECTANGLE_NV,
};

VkPolygonMode Convert(PolygonMode mode);

enum class PrimitiveTopology
{
	POINT_LIST,
	LINE_LIST,
	LINE_STRIP,
	TRIANGLE_LIST,
	TRIANGLE_STRIP,
	TRIANGLE_FAN,
	LINE_LIST_WITH_ADJACENCY,
	LINE_STRIP_WITH_ADJACENCY,
	TRIANGLE_LIST_WITH_ADJACENCY,
	TRIANGLE_STRIP_WITH_ADJACENCY,
	PATCH_LIST,
};

VkPrimitiveTopology Convert(PrimitiveTopology topology);

enum class TextureType : int
{
	UNDEFINED = 0,

	TEXTURE2D,
	CUBE
};

enum class Filter
{
	UNDEFINED = 0,

	NEAREST,
	LINEAR
};

VkFilter Convert(Filter filter);

enum class AttachmentType
{
	UNDEFINED = 0,

	COLOR,
	DEPTH,
	DEPTH_STNCIL
};

enum class MeshPrimitiveType : int
{
	NONE = 0,

	CUBE,
	SPHERE
};
