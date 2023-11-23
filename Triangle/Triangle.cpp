#include "Core.h"

class Triangle : public Application
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Color;
	};
protected:
	virtual void OnInit() override
	{
		Shader::CompileShaders(GetProjectDirectory() + "/Shaders/");

		Layout layout;
		layout.Add<glm::vec3>("inPosition");
		layout.Add<glm::vec3>("inColor");

		std::vector<Triangle::Vertex> vertices = {
			{.Position = {  1.0f,  1.0f, 0.0f }, .Color = { 1.0f, 0.0f, 0.0f } },
			{.Position = { -1.0f,  1.0f, 0.0f }, .Color = { 0.0f, 1.0f, 0.0f } },
			{.Position = {  0.0f, -1.0f, 0.0f }, .Color = { 0.0f, 0.0f, 1.0f } } };

		std::vector<uint32_t> indices = { 0, 1, 2 };

		m_VertexBuffer = Buffer::CreateVertex(vertices.size() * sizeof(Triangle::Vertex), vertices.data());
		m_IndexBuffer = Buffer::CreateIndex(indices.size() * sizeof(uint32_t), indices.data());

		PipelineDescription desc;

		desc.BufferLayout = &layout;
		desc.ShaderModules = { { StageFlag::VERTEX, "Shaders/Triangle.vert.spv" }, { StageFlag::FRAGMENT, "Shaders/Triangle.frag.spv" } };
		desc.CullMode = CullMode::NONE;

		m_Pipeline = Pipeline::Create(desc);
	}

	virtual void OnUpdate(float dt) override
	{
	}

	virtual void Render(CommandBuffer& commandBuffer) override
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
	Ref<Buffer> m_VertexBuffer;
	Ref<Buffer> m_IndexBuffer;
};

int main(int argc, char** argv)
{
	Scope<Application> app = CreateScope<Triangle>();

	app->Run();

	return 0;
}
