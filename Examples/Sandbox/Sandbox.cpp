#include "Core.h"

#include "Text.h"

#include <unordered_map>

std::vector<SingleText> demoText = {
   {1, {"Hello World", "", "", ""}, 0, 0} };

static constexpr std::array s_AssetsNames = { "Xwing", "Skybox", "Room", "Text" };

class Sandbox : public Application
{
	struct UBO
	{
		alignas(16) glm::mat4 MVP;
		alignas(16) glm::mat4 Model;
		alignas(16) glm::mat4 Normal;
	};

	struct GUBO
	{
		alignas(16) glm::vec3 LightDir;
		alignas(16) glm::vec4 LightColor;
		alignas(16) glm::vec3 CameraPosition;
	};
private:
	virtual void OnInit() override
	{
		const auto& [width, height] = Application::GetSize();
		m_Camera = Camera(float(width) / float(height));

		ShaderCompiler::CompileWithValidator(GetProjectDirectory() + "/Shaders/");

		// Xwing
		{
			const auto xWingAssetName = s_AssetsNames[0];

			m_Meshes[xWingAssetName] = Mesh::Create("Models/Xwing.obj");
			m_Textures[xWingAssetName] = Texture::Create("Textures/XwingColors.png");

			m_UniformBuffers.insert({ xWingAssetName, GBuffer::CreateUniform(sizeof(Sandbox::UBO)) });
			m_UniformBuffers.insert({ xWingAssetName, GBuffer::CreateUniform(sizeof(Sandbox::GUBO)) });

			PipelineDescription desc;

			desc.ShaderModules = { { StageFlag::VERTEX, "Shaders/Phong.vert.spv" }, { StageFlag::FRAGMENT, "Shaders/Phong.frag.spv" } };

			m_Pipelines[xWingAssetName] = Pipeline::Create(desc);

			m_DescriptorSets[xWingAssetName] = DescriptorSet::Create({ m_Pipelines[xWingAssetName]->GetShader() });

			uint32_t binding = 0;
			auto range = m_UniformBuffers.equal_range(xWingAssetName);
			for (auto it = range.first; it != range.second; it++)
			{
				m_DescriptorSets[xWingAssetName]->SetBuffer(binding, *(it->second));
				binding += 2;
			}

			m_DescriptorSets[xWingAssetName]->SetTexture(1, static_cast<const Texture&>(*m_Textures[xWingAssetName]));
		}

		// Skybox
		{
			const auto skyboxAssetName = s_AssetsNames[1];

			m_Skybox = Skybox::Create("Models/SkyboxCube.obj",
				{ "Textures/sky/right.png", "Textures/sky/left.png",
				"Textures/sky/top.png",   "Textures/sky/bottom.png",
				"Textures/sky/front.png", "Textures/sky/back.png" });

			const auto& skyboxUniform = m_UniformBuffers.insert({ skyboxAssetName, GBuffer::CreateUniform(sizeof(Sandbox::UBO)) });

			PipelineDescription desc;

			desc.ShaderModules = { { StageFlag::VERTEX, "Shaders/Skybox.vert.spv" }, { StageFlag::FRAGMENT, "Shaders/Skybox.frag.spv" } };
			desc.CompareOp = CompareOp::LESS_OR_EQUAL;

			m_Pipelines[skyboxAssetName] = Pipeline::Create(desc);

			m_DescriptorSets[skyboxAssetName] = DescriptorSet::Create({ m_Pipelines[skyboxAssetName]->GetShader() });

			m_DescriptorSets[skyboxAssetName]->SetBuffer(0, *(skyboxUniform->second));
			m_DescriptorSets[skyboxAssetName]->SetTexture(1, static_cast<const Texture&>(m_Skybox->GetTexture()));
		}

		// Room
		{
			const auto roomAssetName = s_AssetsNames[2];

			m_Meshes[roomAssetName] = Mesh::Create("Models/VikingRoom.obj");
			m_Textures[roomAssetName] = Texture::Create("Textures/VikingRoom.png");

			m_UniformBuffers.insert({ roomAssetName, GBuffer::CreateUniform(sizeof(Sandbox::UBO)) });
			m_UniformBuffers.insert({ roomAssetName, GBuffer::CreateUniform(sizeof(Sandbox::GUBO)) });

			PipelineDescription desc;

			desc.ShaderModules = { { StageFlag::VERTEX, "Shaders/Phong.vert.spv" }, { StageFlag::FRAGMENT, "Shaders/Phong.frag.spv" } };
			desc.CullMode = CullMode::NONE;

			m_Pipelines[roomAssetName] = Pipeline::Create(desc);

			m_DescriptorSets[roomAssetName] = DescriptorSet::Create({ m_Pipelines[roomAssetName]->GetShader() });

			uint32_t binding = 0;
			auto range = m_UniformBuffers.equal_range(roomAssetName);
			for (auto it = range.first; it != range.second; it++)
			{
				m_DescriptorSets[roomAssetName]->SetBuffer(binding, *(it->second));
				binding += 2;
			}

			m_DescriptorSets[roomAssetName]->SetTexture(1, static_cast<const Texture&>(*m_Textures[roomAssetName]));
		}

		// Text
		{
			const auto textAssetName = s_AssetsNames[3];

			std::vector<Vertex> textVertices;
			std::vector<uint32_t> textIndices;
			CreateTextMesh(demoText, textVertices, textIndices);

			m_Meshes[textAssetName] = Mesh::Create(textVertices, textIndices);
			m_Textures[textAssetName] = Texture::Create("Textures/Fonts.png");

			const auto& textUniform = m_UniformBuffers.insert({ textAssetName, GBuffer::CreateUniform(sizeof(Sandbox::UBO)) });

			PipelineDescription desc;

			desc.ShaderModules = { { StageFlag::VERTEX, "Shaders/Text.vert.spv" }, { StageFlag::FRAGMENT, "Shaders/Text.frag.spv" } };
			desc.CompareOp = CompareOp::LESS_OR_EQUAL;
			desc.CullMode = CullMode::NONE;
			desc.EnableTransparency = true;

			m_Pipelines[textAssetName] = Pipeline::Create(desc);

			m_DescriptorSets[textAssetName] = DescriptorSet::Create({ m_Pipelines[textAssetName]->GetShader() });

			m_DescriptorSets[textAssetName]->SetBuffer(0, *(textUniform->second));
			m_DescriptorSets[textAssetName]->SetTexture(1, static_cast<const Texture&>(*m_Textures[textAssetName]));
		}
	}

	virtual void OnUpdate(float dt) override
	{
		m_Angle += 1.0f * dt;

		m_Camera.OnUpdate(dt);

		UpdateModels();

		UpdateUniformBuffers();
	}

	virtual void OnRender(CommandBuffer& commandBuffer) override
	{
		for (size_t i = 0; i < s_AssetsNames.size(); i++)
		{
			const auto assetName = s_AssetsNames[i];
			commandBuffer.BindPipeline(*m_Pipelines[assetName]);
			commandBuffer.BindDescriptorSet(*(m_DescriptorSets.find(assetName)->second));

			if (i == 1)
			{
				const auto& skyboxMesh = m_Skybox->GetMesh();
				commandBuffer.BindVertexBuffer(skyboxMesh.GetVertexBuffer());
				commandBuffer.BindIndexBuffer(skyboxMesh.GetIndexBuffer());
				commandBuffer.DrawIndexed(skyboxMesh.GetIndexCount());
			}
			else
			{
				const auto& mesh = m_Meshes[assetName];
				commandBuffer.BindVertexBuffer(mesh->GetVertexBuffer());
				commandBuffer.BindIndexBuffer(mesh->GetIndexBuffer());
				if (i != 3)
					commandBuffer.DrawIndexed(m_Meshes[assetName]->GetIndexCount());
				else
					commandBuffer.DrawIndexed(static_cast<uint32_t>(demoText[0].len), static_cast<uint32_t>(demoText[0].start));
			}

		}
	}

	virtual void OnShutdown() override
	{
		m_Pipelines.clear();
		m_DescriptorSets.clear();
		m_Textures.clear();
		m_Meshes.clear();
		m_UniformBuffers.clear();

		m_Skybox.reset();
	}

	virtual void OnEvent(Event& event) override
	{
	}
private:
	void UpdateModels()
	{
		// Xwing
		{
			const auto xWingAssetName = s_AssetsNames[0];
			auto& model = m_Models[xWingAssetName];

			model = glm::identity<glm::mat4>();
			//model = glm::scale(m_Model, glm::vec3(5.0f, 5.0f, 5.0f));
			model = glm::translate(model, glm::vec3(-5.0f, 0.0f, 0.0f));
			model = glm::rotate(model, m_Angle, glm::vec3(0.0f, 1.0f, 0.0f));
		}

		// Room
		{
			const auto roomAssetName = s_AssetsNames[2];
			auto& model = m_Models[roomAssetName];

			model = glm::identity<glm::mat4>();
			model = glm::scale(model, glm::vec3(2.5f, 2.5f, 2.5f));
			model = glm::translate(model, glm::vec3(2.5f, 0.0f, 0.0f));
			model = glm::rotate(model, -m_Angle, glm::vec3(0.0f, 1.0f, 0.0f));
		}
	}

	void UpdateUniformBuffers()
	{
		const auto& cameraPosition = m_Camera.GetPosition();
		const auto& cameraViewProjection = m_Camera.GetViewProjection();

		// Xwing
		{
			const auto xWingAssetName = s_AssetsNames[0];

			Sandbox::UBO ubo = {};
			ubo.Model = m_Models[xWingAssetName];
			ubo.MVP = cameraViewProjection * ubo.Model;
			ubo.Normal = glm::inverseTranspose(ubo.Model);

			Sandbox::GUBO gubo = {};
			gubo.LightDir = glm::vec3(glm::cos(glm::radians(135.0f)), glm::sin(glm::radians(135.0f)), 0.0f);
			gubo.LightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			gubo.CameraPosition = cameraPosition;


			uint32_t binding = 0;
			auto range = m_UniformBuffers.equal_range(xWingAssetName);
			for (auto it = range.first; it != range.second; it++)
			{
				Ref<GBuffer> buffer = it->second;
				(binding == 0) ? buffer->SetData(&ubo) : buffer->SetData(&gubo);

				binding += 2;
			}
		}

		// Skybox
		{
			const auto skyboxAssetName = s_AssetsNames[1];

			Sandbox::UBO sbubo = {};
			sbubo.MVP = m_Camera.GetProjection() * glm::transpose(glm::mat4(m_Camera.GetRotation()));
			sbubo.Model = glm::mat4(1.0f);
			sbubo.Normal = glm::mat4(1.0f);

			m_UniformBuffers.find(skyboxAssetName)->second->SetData(&sbubo);
		}

		// Room
		{
			const auto roomAssetName = s_AssetsNames[2];

			Sandbox::UBO ubo = {};
			ubo.Model = m_Models[roomAssetName];
			ubo.MVP = m_Camera.GetViewProjection() * ubo.Model;
			ubo.Normal = glm::inverseTranspose(ubo.Model);

			Sandbox::GUBO gubo = {};
			gubo.LightDir = glm::vec3(glm::cos(glm::radians(135.0f)), glm::sin(glm::radians(135.0f)), 0.0f);
			gubo.LightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			gubo.CameraPosition = cameraPosition;

			uint32_t binding = 0;
			auto range = m_UniformBuffers.equal_range(roomAssetName);
			for (auto it = range.first; it != range.second; it++)
			{
				Ref<GBuffer> buffer = it->second;
				(0 == binding) ? buffer->SetData(&ubo) : buffer->SetData(&gubo);

				binding += 2;
			}
		}
	}
private:
	float m_Angle = 0.0f;

	Camera m_Camera;

	Ref<Skybox> m_Skybox;

	std::unordered_map<std::string, glm::mat4> m_Models;
	std::unordered_map<std::string, Ref<DescriptorSet>> m_DescriptorSets;
	std::unordered_map<std::string, Ref<Texture>> m_Textures;
	std::unordered_map<std::string, Ref<Pipeline>> m_Pipelines;
	std::unordered_map<std::string, Ref<Mesh>> m_Meshes;
	std::unordered_multimap<std::string, Ref<GBuffer>> m_UniformBuffers;
};

int main(int argc, char** argv)
{
	Sandbox app;

	app.Run();

	return 0;
}
