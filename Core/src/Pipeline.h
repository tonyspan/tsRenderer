#pragma once

#include "Base.h"

#include "VK.h"

#include "Enums.h"

#include <map>
#include <filesystem>

class Layout;

struct PipelineDescription
{
	std::map<StageFlag, std::filesystem::path> ShaderModules;
	VkDescriptorSetLayout DescSetLayout = nullptr;
	const Layout* BufferLayout = nullptr;

	CompareOp CompareOp = CompareOp::LESS;
	PolygonMode PolygonMode = PolygonMode::FILL;
	CullMode CullMode = CullMode::BACK;
	PrimitiveTopology Topology = PrimitiveTopology::TRIANGLE_LIST;
	bool EnableTransparency = false;
};

class Pipeline : public Handle<VkPipeline, VkPipelineLayout>
{
public:
	static Ref<Pipeline> Create(const PipelineDescription& desc);

	Pipeline(const PipelineDescription& desc);
	~Pipeline();
private:
	void CreatePipeline(const PipelineDescription& desc);
};