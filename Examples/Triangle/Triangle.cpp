#include "Core.h"

class Triangle : public Application
{
protected:
	virtual void OnInit() override
	{
		std::array shaderCode = {
				R"(
				#version 450
				layout (location = 0) in vec3 inPosition;
				layout (location = 1) in vec3 inColor;
				layout (location = 0) out vec3 outColor;

				void main()
				{
					outColor = inColor;
					gl_Position = vec4(inPosition.xyz, 1.0);
				})",
				R"(
				#version 450
				layout (location = 0) in vec3 inColor;
				layout (location = 0) out vec4 outColor;

				void main() 
				{
					outColor = vec4(inColor, 1.0);
				})"
		};

		Buffer vertCode;
		Buffer fragCode;

		ShaderCompiler::Compile(vertCode, StageFlag::VERTEX, shaderCode[0]);
		ShaderCompiler::Compile(fragCode, StageFlag::FRAGMENT, shaderCode[1]);

		auto shader = Shader::Create({ { StageFlag::VERTEX, vertCode }, { StageFlag::FRAGMENT, fragCode } });

		vertCode.Release();
		fragCode.Release();

		std::vector<::SimpleVertex> vertices = {
			{.Position = {  1.0f,  1.0f, 0.0f }, .Color = { 1.0f, 0.0f, 0.0f } },
			{.Position = { -1.0f,  1.0f, 0.0f }, .Color = { 0.0f, 1.0f, 0.0f } },
			{.Position = {  0.0f, -1.0f, 0.0f }, .Color = { 0.0f, 0.0f, 1.0f } } };

		std::vector<uint32_t> indices = { 0, 1, 2 };

		m_VertexBuffer = GBuffer::CreateVertex(vertices.size() * sizeof(::SimpleVertex), vertices.data());
		m_IndexBuffer = GBuffer::CreateIndex(indices.size() * sizeof(uint32_t), static_cast<uint32_t>(indices.size()), indices.data());

		m_Pipeline = Pipeline::Create(PipelineDescription{}, shader);
	}

	virtual void OnUpdate(float dt) override
	{
	}

	virtual void OnRender(CommandBuffer& commandBuffer) override
	{
		commandBuffer.BindPipeline(*m_Pipeline);

		commandBuffer.BindVertexBuffer(*m_VertexBuffer);
		commandBuffer.BindIndexBuffer(*m_IndexBuffer);

		commandBuffer.DrawIndexed(3);
	}

	virtual void OnShutdown() override
	{
		m_Pipeline.reset();
		m_VertexBuffer.reset();
		m_IndexBuffer.reset();
	}

	virtual void OnEvent(Event& event) override
	{
	}
private:
	Ref<Pipeline> m_Pipeline;
	Ref<GBuffer> m_VertexBuffer;
	Ref<GBuffer> m_IndexBuffer;
};

int main(int argc, char** argv)
{
	Triangle app;

	app.Run();

	return 0;
}
