#include "Core.h"

#include <imgui.h>

class Cube : public Application
{
	struct UBO
	{
		alignas(16) glm::mat4 View;
		alignas(16) glm::mat4 Projection;
	};
protected:
	virtual void OnInit() override
	{
		const auto& [width, height] = Application::GetSize();
		m_Camera = Camera(float(width) / float(height));

		ShaderCompiler::CompileWithValidator(GetProjectDirectory() + "/Shaders/");

		std::vector<::Vertex> vertices = {
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

		m_VertexBuffer = GBuffer::CreateVertex(vertices.size() * sizeof(::Vertex), vertices.data());
		m_UniformBuffer = GBuffer::CreateUniform(sizeof(Cube::UBO));

		m_Texture = Texture::Create("Textures/container.jpg");

		PipelineDescription desc;

		desc.ShaderModules = { { StageFlag::VERTEX, "Shaders/Cube.vert.spv" }, { StageFlag::FRAGMENT, "Shaders/Cube.frag.spv" } };
		desc.CullMode = CullMode::BACK;
		desc.EnableTransparency = true;

		m_Pipeline = Pipeline::Create(desc);

		m_DS = DescriptorSet::Create({ m_Pipeline->GetShader() });

		m_DS->SetBuffer("ubo", *m_UniformBuffer);
		m_DS->SetTexture("uTexture", *m_Texture);
	}

	virtual void OnUpdate(float dt) override
	{
		m_Camera.OnUpdate(dt);

		glm::mat4 model = glm::scale(glm::identity<glm::mat4>(), glm::vec3(5.0f, 5.0f, 5.0f));
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 45.0f, { 0.0f, 1.0f, 0.5f });
		m_Model = model;

		Cube::UBO ubo = {};

		ubo.View = m_Camera.GetView();
		ubo.Projection = m_Camera.GetProjection();

		m_UniformBuffer->SetData(&ubo);
	}

	virtual void OnRender(CommandBuffer& commandBuffer) override
	{
		commandBuffer.BindPipeline(*m_Pipeline);

		static float opacity = 0.5f;
		ImGui::SliderFloat("Opacity", &opacity, 0.0f, 1.0f, "%.2f");

		commandBuffer.PushConstant("constants.Model", m_Model);
		commandBuffer.PushConstant("constants.Opacity", opacity);

		commandBuffer.BindVertexBuffer(*m_VertexBuffer);
		commandBuffer.BindDescriptorSet(*m_DS);

		commandBuffer.Draw(36);
	}

	virtual void OnShutdown() override
	{
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

	glm::mat4 m_Model;
};

int main(int argc, char** argv)
{
	Cube app;

	app.Run();

	return 0;
}
