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
		Shader::CompileShaders(GetProjectDirectory() + "/Shaders/");

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

		m_VertexBuffer = Buffer::CreateVertex(vertices.size() * sizeof(Cube::Vertex), vertices.data());

		m_Texture = Texture2D::Create("Textures/container.jpg");

		auto dsl = DescriptorSetLayout::Create({
			DescriptorSetLayoutBinding{ 0, DescriptorType::UNIFORM_BUFFER, StageFlag::VERTEX },
			DescriptorSetLayoutBinding{ 1, DescriptorType::COMBINED_IMAGE_SAMPLER, StageFlag::FRAGMENT }
			});

		std::vector<DescriptorSetElement> elements =
		{
			DescriptorSetElement{ 0, DescriptorSetElement::Type::UNIFORM, sizeof(UBO), (Texture*)nullptr },
			DescriptorSetElement{ 1, DescriptorSetElement::Type::SAMPLER, 0, m_Texture.get() },
		};

		{
			DescriptorSetDescription desc;

			desc.DescSetLayout = dsl.get();
			desc.Elements = elements;

			m_DS = DescriptorSet::Create(desc);
		}

		PipelineDescription desc;

		desc.DescSetLayout = dsl.get();
		desc.BufferLayout = &layout;
		desc.ShaderModules = { { StageFlag::VERTEX, "Shaders/Cube.vert.spv" }, { StageFlag::FRAGMENT, "Shaders/Cube.frag.spv" } };
		desc.CullMode = CullMode::NONE;

		m_Pipeline = Pipeline::Create(desc);

		dsl.reset();
	}

	virtual void OnUpdate(float dt) override
	{
		UpdateCamera(dt);

		glm::mat4 model = glm::scale(glm::identity<glm::mat4>(), glm::vec3(5.0f, 5.0f, 5.0f));
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 45.0f, { 0.0f, 1.0f, 0.5f });

		UBO ubo = {};
		ubo.MVP = m_Projection * m_View * model;
		m_DS->Update(0, &ubo, sizeof(ubo));
	}

	virtual void Render(CommandBuffer& commandBuffer) override
	{
		commandBuffer.BindPipeline(*m_Pipeline);

		commandBuffer.BindVertexBuffer(*m_VertexBuffer);

		commandBuffer.BindDescriptorSet(m_Pipeline->GetLayout(), m_DS->GetDescriptorSet());

		commandBuffer.Draw(36);
	}

	virtual void OnShutdown() override
	{
		m_Pipeline.reset();

		m_Pipeline.reset();
		m_DS.reset();

		m_Texture.reset();

		m_VertexBuffer.reset();
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
private:
	glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 10.0f };
	glm::quat m_CameraRotation{ 1.0f, 0.0f, 0.0f, 0.0f };

	glm::mat4 m_View;
	glm::mat4 m_Projection;

	Ref<Pipeline> m_Pipeline;
	Ref<Buffer> m_VertexBuffer;
	Ref<Texture2D> m_Texture;
	Ref<DescriptorSet> m_DS;
};

int main(int argc, char** argv)
{
	Scope<Application> app = CreateScope<Cube>();

	app->Run();

	return 0;
}
