#include "Camera.h"

#include "Input.h"

Camera::Camera(float aspectRatio, float fovYdegrees, float near, float far)
{
	m_Projection = glm::perspective(glm::radians(fovYdegrees), aspectRatio, near, far);
	m_Projection[1][1] *= -1.0f;
}

void Camera::OnUpdate(float dt)
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
}

const glm::mat4& Camera::GetProjection() const
{
	return m_Projection;
}

const glm::mat4& Camera::GetView() const
{
	return m_View;
}

const glm::mat4 Camera::GetViewProjection() const
{
	return GetProjection() * GetView();
}

const glm::vec3& Camera::GetPosition() const
{
	return m_CameraPosition;
}

const glm::quat& Camera::GetRotation() const
{
	return m_CameraRotation;
}
