#pragma once

#include "Base.h"

#include "VK.h"

#include "Enums.h"

#include <vector>
#include <filesystem>

class Shader;

struct PipelineDescription
{
	std::vector<std::pair<StageFlag, std::filesystem::path>> ShaderModules;
	CompareOp CompareOp = CompareOp::LESS;
	PolygonMode PolygonMode = PolygonMode::FILL;
	float LineWidth = 1.0f;
	CullMode CullMode = CullMode::NONE;
	PrimitiveTopology Topology = PrimitiveTopology::TRIANGLE_LIST;
	bool EnableTransparency = false;
	bool EnableDynamicStates = false;
};

class Pipeline : public Handle<VkPipeline, VkPipelineLayout>
{
public:
	static Ref<Pipeline> Create(const PipelineDescription& desc);
	static Ref<Pipeline> Create(const PipelineDescription& desc, Ref<Shader> shader);

	Pipeline(const PipelineDescription& desc, Ref<Shader> shader);
	~Pipeline();

	WeakRef<Shader> GetShader() const;

	const PipelineDescription& GetDescription() const;
private:
	void CreatePipelineLayout();
	void CreatePipeline();
private:
	PipelineDescription m_Description;

	Ref<Shader> m_Shader = nullptr;
};