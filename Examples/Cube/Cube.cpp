#include "Core.h"

class Cube : public Application
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
	};

	struct UBO
	{
		alignas(16) glm::mat4 MVP;
	};
protected:
	virtual void OnInit() override
	{
		const auto& [width, height] = Application::GetSize();
		m_Camera = Camera(float(width) / float(height));

		ShaderCompiler::CompileWithValidator(GetProjectDirectory() + "/Shaders/");

		Layout layout;
		layout.Add<glm::vec3>("inPosition");
		layout.Add<glm::vec2>("inTexCoord");

		std::vector<Cube::Vertex> vertices = {
				 {.Position = { -0.5f, -0.5f, -0.5f }, .TexCoord = { 0.0f, 0.0f } },
				 {.Position = {  0.5f,  0.5f, -0.5f }, .TexCoord = { 1.0f, 1.0f } },
				 {.Position = {  0.5f, -0.5f, -0.5f }, .TexCoord = { 1.0f, 0.0f } },
				 {.Position = {  0.5f,  0.5f, -0.5f }, .TexCoord = { 1.0f, 1.0f } },
				 {.Position = { -0.5f, -0.5f, -0.5f }, .TexCoord = { 0.0f, 0.0f } },
				 {.Position = { -0.5f,  0.5f, -0.5f }, .TexCoord = { 0.0f, 1.0f } },

				 {.Position = { -0.5f, -0.5f,  0.5f }, .TexCoord = { 0.0f, 0.0f } },
				 {.Position = {  0.5f, -0.5f,  0.5f }, .TexCoord = { 1.0f, 0.0f } },
				 {.Position = {  0.5f,  0.5f,  0.5f }, .TexCoord = { 1.0f, 1.0f } },
				 {.Position = {  0.5f,  0.5f,  0.5f }, .TexCoord = { 1.0f, 1.0f } },
				 {.Position = { -0.5f,  0.5f,  0.5f }, .TexCoord = { 0.0f, 1.0f } },
				 {.Position = { -0.5f, -0.5f,  0.5f }, .TexCoord = { 0.0f, 0.0f } },

				 {.Position = { -0.5f,  0.5f,  0.5f }, .TexCoord = { 1.0f, 0.0f } },
				 {.Position = { -0.5f,  0.5f, -0.5f }, .TexCoord = { 1.0f, 1.0f } },
				 {.Position = { -0.5f, -0.5f, -0.5f }, .TexCoord = { 0.0f, 1.0f } },
				 {.Position = { -0.5f, -0.5f, -0.5f }, .TexCoord = { 0.0f, 1.0f } },
				 {.Position = { -0.5f, -0.5f,  0.5f }, .TexCoord = { 0.0f, 0.0f } },
				 {.Position = { -0.5f,  0.5f,  0.5f }, .TexCoord = { 1.0f, 0.0f } },

				 {.Position = { 0.5f,  0.5f,  0.5f }, .TexCoord = { 1.0f, 0.0f } },
				 {.Position = { 0.5f, -0.5f, -0.5f }, .TexCoord = { 0.0f, 1.0f } },
				 {.Position = { 0.5f,  0.5f, -0.5f }, .TexCoord = { 1.0f, 1.0f } },
				 {.Position = { 0.5f, -0.5f, -0.5f }, .TexCoord = { 0.0f, 1.0f } },
				 {.Position = { 0.5f,  0.5f,  0.5f }, .TexCoord = { 1.0f, 0.0f } },
				 {.Position = { 0.5f, -0.5f,  0.5f }, .TexCoord = { 0.0f, 0.0f } },

				 {.Position = { -0.5f, -0.5f, -0.5f }, .TexCoord = { 0.0f, 1.0f } },
				 {.Position = {  0.5f, -0.5f, -0.5f }, .TexCoord = { 1.0f, 1.0f } },
				 {.Position = {  0.5f, -0.5f,  0.5f }, .TexCoord = { 1.0f, 0.0f } },
				 {.Position = {  0.5f, -0.5f,  0.5f }, .TexCoord = { 1.0f, 0.0f } },
				 {.Position = { -0.5f, -0.5f,  0.5f }, .TexCoord = { 0.0f, 0.0f } },
				 {.Position = { -0.5f, -0.5f, -0.5f }, .TexCoord = { 0.0f, 1.0f } },

				 {.Position = { -0.5f,  0.5f, -0.5f }, .TexCoord = { 0.0f, 1.0f } },
				 {.Position = {  0.5f,  0.5f,  0.5f }, .TexCoord = { 1.0f, 0.0f } },
				 {.Position = {  0.5f,  0.5f, -0.5f }, .TexCoord = { 1.0f, 1.0f } },
				 {.Position = {  0.5f,  0.5f,  0.5f }, .TexCoord = { 1.0f, 0.0f } },
				 {.Position = { -0.5f,  0.5f, -0.5f }, .TexCoord = { 0.0f, 1.0f } },
				 {.Position = { -0.5f,  0.5f,  0.5f }, .TexCoord = { 0.0f, 0.0f } }
		};

		m_VertexBuffer = GBuffer::CreateVertex(vertices.size() * sizeof(Cube::Vertex), vertices.data());
		m_UniformBuffer = GBuffer::CreateUniform(sizeof(Cube::UBO));

		m_Texture = Texture::Create("Textures/container.jpg");

		m_DS = DescriptorSet::Create({
			{.Name = "UBO", .Binding = 0, .Type = DescriptorType::UNIFORM_BUFFER, .Stage = StageFlag::VERTEX },
			{.Name = "uTexture", .Binding = 1, .Type = DescriptorType::COMBINED_IMAGE_SAMPLER, .Stage = StageFlag::FRAGMENT }
			});

		m_DS->SetBuffer(0, *m_UniformBuffer);
		m_DS->SetTexture(1, *m_Texture);

		PipelineDescription desc;

		desc.DescSetLayout = m_DS->GetLayout();
		desc.BufferLayout = &layout;
		desc.ShaderModules = { { StageFlag::VERTEX, "Shaders/Cube.vert.spv" }, { StageFlag::FRAGMENT, "Shaders/Cube.frag.spv" } };
		desc.CullMode = CullMode::NONE;

		m_Pipeline = Pipeline::Create(desc);
	}

	virtual void OnUpdate(float dt) override
	{
		m_Camera.OnUpdate(dt);

		glm::mat4 model = glm::scale(glm::identity<glm::mat4>(), glm::vec3(5.0f, 5.0f, 5.0f));
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 45.0f, { 0.0f, 1.0f, 0.5f });

		Cube::UBO ubo = {};
		ubo.MVP = m_Camera.GetViewProjection() * model;
		m_UniformBuffer->SetData(&ubo);
	}

	virtual void Render(CommandBuffer& commandBuffer) override
	{
		commandBuffer.BindPipeline(*m_Pipeline);

		commandBuffer.BindVertexBuffer(*m_VertexBuffer);

		commandBuffer.BindDescriptorSet(*m_DS);

		commandBuffer.Draw(36);
	}

	virtual void OnShutdown() override
	{
		m_Pipeline.reset();

		m_Pipeline.reset();
		m_DS.reset();

		m_Texture.reset();

		m_VertexBuffer.reset();
		m_UniformBuffer.reset();
	}

	virtual void OnEvent(Event& event) override
	{
	}
private:
	Camera m_Camera;

	Ref<Pipeline> m_Pipeline;
	Ref<GBuffer> m_VertexBuffer;
	Ref<GBuffer> m_UniformBuffer;
	Ref<Texture> m_Texture;

	Ref<DescriptorSet> m_DS;
};

int main(int argc, char** argv)
{
	Scope<Application> app = CreateScope<Cube>();

	app->Run();

	return 0;
}
