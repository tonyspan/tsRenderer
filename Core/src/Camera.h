#pragma once

#include "Input.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct Camera
{
	Camera(float aspectRatio = 1.77f /* 16:9 */, float fovYdegrees = 70.0f, float near = 0.1f, float far = 1000.0f);
	~Camera() = default;

	void OnUpdate(float dt);

	const glm::mat4& GetProjection() const;
	const glm::mat4& GetView() const;
	const glm::mat4 GetViewProjection() const;
	const glm::vec3& GetPosition() const;
	const glm::quat& GetRotation() const;
private:
	glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 10.0f };
	glm::quat m_CameraRotation = { 1.0f, 0.0f, 0.0f, 0.0f };

	glm::mat4 m_View = glm::mat4(1.0f);
	glm::mat4 m_Projection = glm::mat4(1.0f);
};
