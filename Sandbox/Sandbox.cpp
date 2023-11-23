#include "Core.h"

#include "Text.h"

std::vector<SingleText> demoText = {
   {1, {"Hello World", "", "", ""}, 0, 0} };

struct UBO
{
	alignas(16) glm::mat4 MVP;
	alignas(16) glm::mat4 Model;
	alignas(16) glm::mat4 nMat;
};

struct GUBO
{
	alignas(16) glm::vec3 LightDir;
	alignas(16) glm::vec4 LightColor;
	alignas(16) glm::vec3 EyePos; // Camera position
};

class Sandbox : public Application
{
private:
	virtual void OnInit() override
	{
		Shader::CompileShaders(GetProjectDirectory() + "/Shaders/");

#define XWING 1
#if XWING
		m_DSL = DescriptorSetLayout::Create({
			DescriptorSetLayoutBinding{ 0, DescriptorType::UNIFORM_BUFFER, StageFlag::VERTEX },
			DescriptorSetLayoutBinding{ 1, DescriptorType::COMBINED_IMAGE_SAMPLER, StageFlag::FRAGMENT },
			DescriptorSetLayoutBinding{ 2, DescriptorType::UNIFORM_BUFFER, StageFlag::ALL_GRAPHICS }
			});

		{
			PipelineDescription desc;

			desc.DescSetLayout = m_DSL.get();
			desc.ShaderModules = { { StageFlag::VERTEX, "Shaders/Phong.vert.spv" }, { StageFlag::FRAGMENT, "Shaders/Phong.frag.spv" } };

			m_Pipeline = Pipeline::Create(desc);
		}

		m_Mesh = Mesh::Create("Models/Xwing.obj");
		m_Texture = Texture2D::Create("Textures/XwingColors.png");

		{
			std::vector<DescriptorSetElement> elements =
			{
				DescriptorSetElement{ 0, DescriptorSetElement::Type::UNIFORM, sizeof(UBO), (Texture*)nullptr },
				DescriptorSetElement{ 1, DescriptorSetElement::Type::SAMPLER, 0, m_Texture.get() },
				DescriptorSetElement{ 2, DescriptorSetElement::Type::UNIFORM, sizeof(GUBO), (Texture*)nullptr }
			};

			DescriptorSetDescription desc;

			desc.DescSetLayout = m_DSL.get();
			desc.Elements = elements;

			m_DS = DescriptorSet::Create(desc);

		}

		m_SkyboxDSL = DescriptorSetLayout::Create({
			DescriptorSetLayoutBinding{ 0, DescriptorType::UNIFORM_BUFFER, StageFlag::VERTEX },
			DescriptorSetLayoutBinding{ 1, DescriptorType::COMBINED_IMAGE_SAMPLER, StageFlag::FRAGMENT }
			});

		{
			PipelineDescription desc;

			desc.DescSetLayout = m_SkyboxDSL.get();
			desc.ShaderModules = { { StageFlag::VERTEX, "Shaders/Skybox.vert.spv" }, { StageFlag::FRAGMENT, "Shaders/Skybox.frag.spv" } };
			desc.CompareOp = CompareOp::LESS_OR_EQUAL;

			m_SkyboxPipeline = Pipeline::Create(desc);

		}

		m_Skybox = Skybox::Create("Models/SkyBoxCube.obj",
			{ "textures/sky/right.png", "textures/sky/left.png",
			"textures/sky/top.png",   "textures/sky/bot.png",
			"textures/sky/front.png", "textures/sky/back.png" });

		{
			std::vector<DescriptorSetElement> elements =
			{
				DescriptorSetElement{ 0, DescriptorSetElement::Type::UNIFORM, sizeof(UBO), (Texture*)nullptr },
				DescriptorSetElement{ 1, DescriptorSetElement::Type::SAMPLER, 0, &m_Skybox->GetTexture() }
			};

			DescriptorSetDescription desc;

			desc.DescSetLayout = m_SkyboxDSL.get();
			desc.Elements = elements;

			m_SkyboxDS = DescriptorSet::Create(desc);

		}

		m_TextDSL = DescriptorSetLayout::Create({
			DescriptorSetLayoutBinding{ 0, DescriptorType::UNIFORM_BUFFER, StageFlag::VERTEX },
			DescriptorSetLayoutBinding{ 1, DescriptorType::COMBINED_IMAGE_SAMPLER, StageFlag::FRAGMENT }
			});

		{
			PipelineDescription desc;

			desc.DescSetLayout = m_TextDSL.get();
			desc.ShaderModules = { { StageFlag::VERTEX, "Shaders/Text.vert.spv" }, { StageFlag::FRAGMENT, "Shaders/Text.frag.spv" } };
			desc.CompareOp = CompareOp::LESS_OR_EQUAL;
			desc.CullMode = CullMode::NONE;
			desc.TransparencyEnabled = true;

			m_TextPipeline = Pipeline::Create(desc);
		}

		std::vector<Vertex> textVertices;
		std::vector<uint32_t> textIndices;
		CreateTextMesh(demoText, textVertices, textIndices);

		m_TextMesh = Mesh::Create(textVertices, textIndices);

		m_TextTexture = Texture2D::Create("Textures/Fonts.png");

		{
			std::vector<DescriptorSetElement> elements =
			{
				DescriptorSetElement{ 0, DescriptorSetElement::Type::UNIFORM, sizeof(UBO), (Texture*)nullptr },
				DescriptorSetElement{ 1, DescriptorSetElement::Type::SAMPLER, 0, m_TextTexture.get() }
			};

			DescriptorSetDescription desc;

			desc.DescSetLayout = m_TextDSL.get();
			desc.Elements = elements;

			m_TextDS = DescriptorSet::Create(desc);

		}
#else
		m_DSL = DescriptorSetLayout::Create({
			DescriptorSetLayoutBinding{ 0, DescriptorType::UNIFORM_BUFFER,  StageFlag::VERTEX },
			DescriptorSetLayoutBinding{ 1, DescriptorType::COMBINED_IMAGE_SAMPLER,  StageFlag::FRAGMENT },
			DescriptorSetLayoutBinding{ 2, DescriptorType::UNIFORM_BUFFER,  StageFlag::ALL_GRAPHICS }
			});

		{
			PipelineDescription desc;

			desc.DescSetLayout = m_DSL.get();
			desc.ShaderModules = { { StageFlag::VERTEX, "Shaders/Phong.vert.spv" }, { StageFlag::FRAGMENT, "Shaders/Phong.frag.spv" } };
			desc.CullMode = CullMode::NONE;

			m_Pipeline = Pipeline::Create(desc);
		}

		m_Mesh = Mesh::Create("Models/VikingRoom.obj");
		m_Texture = Texture2D::Create("Textures/VikingRoom.png");

		{
			std::vector<DescriptorSetElement> elements =
			{
				DescriptorSetElement{ 0, DescriptorSetElement::Type::UNIFORM, sizeof(UBO), (Texture*)nullptr },
				DescriptorSetElement{ 1, DescriptorSetElement::Type::SAMPLER, 0, m_Texture.get() },
				DescriptorSetElement{ 2, DescriptorSetElement::Type::UNIFORM, sizeof(GUBO), (Texture*)nullptr }
			};

			DescriptorSetDescription desc;

			desc.DescSetLayout = m_DSL.get();
			desc.Elements = elements;

			m_DS = DescriptorSet::Create(desc);

		}
#endif
	}

	virtual void OnUpdate(float dt) override
	{
		m_Angle += 1.0f * dt;

		UpdateCamera(dt);

		UpdateModel();

		UpdateDescriptorSets();
	}

	virtual void Render(CommandBuffer& commandBuffer) override
	{
#if XWING
		commandBuffer.BindPipeline(*m_Pipeline);

		commandBuffer.BindVertexBuffer(m_Mesh->GetVertexBuffer());
		commandBuffer.BindIndexBuffer(m_Mesh->GetIndexBuffer());

		commandBuffer.BindDescriptorSet(m_Pipeline->GetLayout(), m_DS->GetDescriptorSet());

		commandBuffer.DrawIndexed(m_Mesh->GetIndexCount());

		commandBuffer.BindPipeline(*m_SkyboxPipeline);

		const auto& skyboxMesh = m_Skybox->GetMesh();

		commandBuffer.BindVertexBuffer(skyboxMesh.GetVertexBuffer());
		commandBuffer.BindIndexBuffer(skyboxMesh.GetIndexBuffer());

		commandBuffer.BindDescriptorSet(m_SkyboxPipeline->GetLayout(), m_SkyboxDS->GetDescriptorSet());

		commandBuffer.DrawIndexed(skyboxMesh.GetIndexCount());

		commandBuffer.BindPipeline(*m_TextPipeline);
		commandBuffer.BindVertexBuffer(m_TextMesh->GetVertexBuffer());
		commandBuffer.BindIndexBuffer(m_TextMesh->GetIndexBuffer());
		commandBuffer.BindDescriptorSet(m_TextPipeline->GetLayout(), m_TextDS->GetDescriptorSet());

		commandBuffer.DrawIndexed(static_cast<uint32_t>(demoText[0].len), static_cast<uint32_t>(demoText[0].start));
#else
		commandBuffer.BindPipeline(*m_Pipeline);

		commandBuffer.BindVertexBuffer(m_Mesh->GetVertexBuffer());
		commandBuffer.BindIndexBuffer(m_Mesh->GetIndexBuffer());

		commandBuffer.BindDescriptorSet(m_Pipeline->GetLayout(), m_DS->GetDescriptorSet());

		commandBuffer.DrawIndexed(m_Mesh->GetIndexCount());
#endif
	}

	virtual void OnShutdown() override
	{
#if XWING
		m_Pipeline.reset();
		m_SkyboxPipeline.reset();
		m_TextPipeline.reset();

		m_DS.reset();
		m_SkyboxDS.reset();
		m_TextDS.reset();

		m_Texture.reset();
		m_TextTexture.reset();

		m_Mesh.reset();
		m_TextMesh.reset();

		m_Skybox.reset();

		m_DSL.reset();
		m_SkyboxDSL.reset();
		m_TextDSL.reset();
#else
		m_DSL.reset();
		m_DS.reset();
		m_Pipeline.reset();
		m_Pipeline.reset();
		m_Mesh.reset();
		m_Texture.reset();
#endif
	}

	virtual void OnEvent(Event& event) override
	{
	}
private:
	void UpdateCamera(float dt)
	{
		constexpr float sensitivity = 5.0f;
		constexpr glm::vec3 upVector = { 0.0f, 1.0f, 0.0f };

		static glm::vec2 oldMousePosition;

		const auto& mousePosition = Input::MousePosition();
		const glm::vec2 mouseDelta = mousePosition - oldMousePosition;
		oldMousePosition = mousePosition;

		glm::vec3 translation = {};

		if (Input::IsKeyPressed(KeyCode::W))
			translation.z -= sensitivity;
		else if (Input::IsKeyPressed(KeyCode::S))
			translation.z += sensitivity;

		if (Input::IsKeyPressed(KeyCode::A))
			translation.x -= sensitivity;
		else if (Input::IsKeyPressed(KeyCode::D))
			translation.x += sensitivity;

		if (Input::IsMousePressed(MouseButton::RMB))
		{
			Input::HideCursor(true);

			const float deltaX = mouseDelta.x * -1.0f;
			const float deltaY = mouseDelta.y;

			m_CameraRotation = glm::angleAxis(deltaX * 0.001f, upVector) * m_CameraRotation;
			auto pitchedRotation = m_CameraRotation * glm::angleAxis(deltaY * 0.001f, glm::vec3{ -1.0f, 0.0f, 0.0f });
			if (glm::dot(pitchedRotation * upVector, upVector) >= 0.0f)
			{
				m_CameraRotation = pitchedRotation;
			}

			m_CameraRotation = glm::normalize(m_CameraRotation);
		}
		else
		{
			Input::HideCursor(false);
		}

		m_CameraPosition += (m_CameraRotation * translation) * dt;

		m_View = glm::translate(glm::transpose(glm::mat4(m_CameraRotation)), -m_CameraPosition);

		const auto& [width, height] = Application::GetSize();

		m_Projection = glm::perspective(glm::radians(70.0f), (float)width / (float)height, 0.1f, 1000.0f);
		m_Projection[1][1] *= -1.0f;
	}

	void UpdateModel()
	{
		m_Model = glm::identity<glm::mat4>();
		m_Model = glm::scale(m_Model, glm::vec3(5.0f, 5.0f, 5.0f));
		m_Model = glm::translate(m_Model, glm::vec3(0.0f, 0.0f, 0.0f));
		m_Model = glm::rotate(m_Model, m_Angle, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	void UpdateDescriptorSets()
	{
#if XWING
		UBO ubo = {};
		ubo.Model = glm::rotate(glm::mat4(1.0f), m_Angle * glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		ubo.MVP = m_Projection * m_View * ubo.Model;
		ubo.nMat = glm::inverseTranspose(ubo.Model);
		m_DS->Update(0, &ubo, sizeof(ubo));

		GUBO gubo = {};
		gubo.LightDir = glm::vec3(cos(glm::radians(135.0f)), sin(glm::radians(135.0f)), 0.0f);
		gubo.LightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		gubo.EyePos = m_CameraPosition;
		m_DS->Update(2, &gubo, sizeof(gubo));

		UBO sbubo = {};
		sbubo.Model = glm::mat4(1.0f);
		sbubo.nMat = glm::mat4(1.0f);
		sbubo.MVP = m_Projection * glm::transpose(glm::mat4(m_CameraRotation));
		m_SkyboxDS->Update(0, &sbubo, sizeof(sbubo));
#else
		UBO ubo = {};
		ubo.Model = m_Model;
		ubo.MVP = m_Projection * m_View * ubo.Model;
		ubo.nMat = glm::inverseTranspose(ubo.Model);
		m_DS->Update(0, &ubo, sizeof(ubo));

		GUBO gubo = {};
		gubo.LightDir = glm::vec3(glm::cos(glm::radians(135.0f)), glm::sin(glm::radians(135.0f)), 0.0f);
		gubo.LightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		gubo.EyePos = m_CameraPosition;
		m_DS->Update(2, &gubo, sizeof(gubo));
#endif
	}
private:
	float m_Angle = 0.0f;

	glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 10.0f };
	glm::quat m_CameraRotation{ 1.0f, 0.0f, 0.0f, 0.0f };

	glm::mat4 m_View;
	glm::mat4 m_Projection;

	glm::mat4 m_Model;

#if XWING
	Ref<DescriptorSetLayout> m_DSL;
	Ref<DescriptorSet> m_DS;
	Ref<Pipeline> m_Pipeline;
	Ref<Mesh> m_Mesh;
	Ref<Texture2D> m_Texture;

	Ref<DescriptorSetLayout> m_SkyboxDSL;
	Ref<DescriptorSet> m_SkyboxDS;
	Ref<Pipeline> m_SkyboxPipeline;
	Ref<Skybox> m_Skybox;

	Ref<DescriptorSetLayout> m_TextDSL;
	Ref<DescriptorSet> m_TextDS;
	Ref<Pipeline> m_TextPipeline;
	Ref<Mesh> m_TextMesh;
	Ref<Texture2D> m_TextTexture;
#else
	Ref<DescriptorSetLayout> m_DSL;
	Ref<DescriptorSet> m_DS;
	Ref<Pipeline> m_Pipeline;
	Ref<Mesh> m_Mesh;
	Ref<Texture2D> m_Texture;
#endif
};

int main(int argc, char** argv)
{
	Scope<Application> app = CreateScope<Sandbox>();

	app->Run();

	return 0;
}
