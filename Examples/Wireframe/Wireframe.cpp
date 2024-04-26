#include "Core.h"

#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>

class Wireframe : public Application
{
	enum class Mode : int { NORMAL = 0, WIREFRAME };

	struct UBO
	{
		glm::mat4 Model;
		glm::mat4 View;
		glm::mat4 Projection;
	};
protected:
	virtual void OnInit() override
	{
		const auto& [width, height] = Application::GetSize();

#define DYNAMIC_VIEWPORT 1
#if DYNAMIC_VIEWPORT
		m_Camera = Camera(float(width) * 0.5f / float(height));
#else
		m_Camera = Camera(float(width) / float(height));
#endif

		ShaderCompiler::CompileWithValidator(GetProjectDirectory() + "/Shaders/");

		m_UniformBuffer = GBuffer::CreateUniform(sizeof(UBO));

		m_Mesh = Mesh::Create(MeshPrimitiveType::CUBE);

		{
			PipelineDescription desc;

			desc.ShaderModules = { { StageFlag::VERTEX, "Shaders/Simple.vert.spv" }, { StageFlag::FRAGMENT, "Shaders/Simple.frag.spv" } };
#if DYNAMIC_VIEWPORT
			desc.EnableDynamicStates = true;
#endif

			m_NormalPipeline = Pipeline::Create(desc);

			m_NormalDS = DescriptorSet::Create({ m_NormalPipeline->GetShader() });
			m_NormalDS->SetBuffer("ubo", *m_UniformBuffer);
		}

		{
			PipelineDescription desc;

			desc.ShaderModules = { { StageFlag::VERTEX, "Shaders/Wireframe.vert.spv" }, { StageFlag::FRAGMENT, "Shaders/Wireframe.frag.spv" } };
			desc.PolygonMode = PolygonMode::LINE;
			desc.LineWidth = 2.0f;
#if DYNAMIC_VIEWPORT
			desc.EnableDynamicStates = true;
#endif

			m_WireframePipeline = Pipeline::Create(desc);

			m_WireframeDS = DescriptorSet::Create({ m_WireframePipeline->GetShader() });
			m_WireframeDS->SetBuffer("ubo", *m_UniformBuffer);
		}
	}

	virtual void OnUpdate(float dt) override
	{
		m_Camera.OnUpdate(dt);

		glm::mat4 model = glm::scale(glm::identity<glm::mat4>(), glm::vec3(5.0f, 5.0f, 5.0f));
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 45.0f, { 0.0f, 1.0f, 0.5f });

		Wireframe::UBO ubo = {};

		ubo.Model = model;
		ubo.View = m_Camera.GetView();
		ubo.Projection = m_Camera.GetProjection();

		m_UniformBuffer->SetData(&ubo);
	}

	virtual void OnRender(CommandBuffer& commandBuffer) override
	{
#if DYNAMIC_VIEWPORT
		const auto& [width, height] = Application::GetSize();

		commandBuffer.BindPipeline(*m_NormalPipeline);
		commandBuffer.SetViewport(width / 2, height);
		commandBuffer.BindDescriptorSet(*m_NormalDS);
		commandBuffer.BindVertexBuffer(m_Mesh->GetVertexBuffer());
		commandBuffer.Draw(m_Mesh->GetIndexCount());

		commandBuffer.BindPipeline(*m_WireframePipeline);
		commandBuffer.SetViewport(width / 2, height, width / 2);
		commandBuffer.BindDescriptorSet(*m_WireframeDS);
		commandBuffer.BindVertexBuffer(m_Mesh->GetVertexBuffer());
		commandBuffer.Draw(m_Mesh->GetIndexCount());
#else
		static int selection = 0;
		ImGui::RadioButton("Normal", &selection, 0);
		ImGui::RadioButton("Wireframe", &selection, 1);

		switch (Wireframe::Mode(selection))
		{
		case Wireframe::Mode::NORMAL:
		{
			commandBuffer.BindPipeline(*m_NormalPipeline);
			commandBuffer.BindDescriptorSet(*m_NormalDS);
			commandBuffer.BindVertexBuffer(m_Mesh->GetVertexBuffer());
			commandBuffer.Draw(m_Mesh->GetIndexCount());

			break;
		}
		case Wireframe::Mode::WIREFRAME:
		{
			commandBuffer.BindPipeline(*m_WireframePipeline);
			commandBuffer.BindDescriptorSet(*m_WireframeDS);
			commandBuffer.BindVertexBuffer(m_Mesh->GetVertexBuffer());
			commandBuffer.Draw(m_Mesh->GetIndexCount());

			break;
		}
		default:
			break;
		}
#endif
	}

	virtual void OnShutdown() override
	{
		m_Mesh.reset();
		m_WireframeDS.reset();
		m_NormalDS.reset();
		m_UniformBuffer.reset();
		m_WireframePipeline.reset();
		m_NormalPipeline.reset();
	}

	virtual void OnEvent(Event& event) override
	{
	}
private:
	Camera m_Camera;

	Ref<Mesh> m_Mesh;
	Ref<GBuffer> m_UniformBuffer;
	Ref<DescriptorSet> m_WireframeDS;
	Ref<Pipeline> m_WireframePipeline;
	Ref<DescriptorSet> m_NormalDS;
	Ref<Pipeline> m_NormalPipeline;
};

int main(int argc, char** argv)
{
	Scope<Application> app = CreateScope<Wireframe>();

	app->Run();

	return 0;
}
