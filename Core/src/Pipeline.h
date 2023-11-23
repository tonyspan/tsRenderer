#pragma once

#include "Base.h"

#include "VK.h"

#include "Enums.h"

#include <map>
#include <string>

class DescriptorSetLayout;
class Layout;

struct PipelineDescription
{
	std::map<StageFlag, std::string> ShaderModules;
	const DescriptorSetLayout* DescSetLayout;
	const Layout* BufferLayout;

	CompareOp CompareOp;
	PolygonMode PolygonMode;
	CullMode CullMode;
	PrimitiveTopology Topology;
	bool TransparencyEnabled = false;

	PipelineDescription();
};

class Pipeline : public Handle<VkPipeline>
{
public:
	static Ref<Pipeline> Create(const PipelineDescription& desc);

	Pipeline(const PipelineDescription& desc);
	~Pipeline();

	VkPipelineLayout GetLayout() const;
private:
	void CreatePipeline(const PipelineDescription& desc);
private:
	VkPipelineLayout m_PipelineLayout = nullptr;
};