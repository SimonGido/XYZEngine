#include "stdafx.h"
#include "MousePicker.h"


namespace XYZ {

	void MousePicker::Update(const EditorCamera& camera)
	{
		m_CurrentRay = calculateMouseRay(camera);
	}

	glm::vec3 MousePicker::calculateMouseRay(const EditorCamera& camera) const
	{
		auto [mx, my] = Input::GetMousePosition();
		glm::vec2 normalizedCoords = getNormalisedDeviceCoordinates(mx, my);
		glm::vec4 clipCoords = glm::vec4(normalizedCoords.x, normalizedCoords.y, -1.0f, 1.0f);
		glm::vec4 eyeCoords = toEyeCoords(camera, clipCoords);
		glm::vec3 worldRay = toWorldCoords(camera, eyeCoords);
		return worldRay;
	}

	glm::vec3 MousePicker::toWorldCoords(const EditorCamera& camera, const glm::vec4& eyeCoords) const
	{
		glm::mat4 invertedView = glm::inverse(camera.GetViewMatrix());
		glm::vec4 rayWorld = invertedView * eyeCoords;
		glm::vec3 mouseRay = glm::vec3(rayWorld.x, rayWorld.y, rayWorld.z);
		mouseRay = glm::normalize(mouseRay);
		return mouseRay;
	}

	glm::vec4 MousePicker::toEyeCoords(const EditorCamera& camera, const glm::vec4& clipCoords) const
	{
		glm::mat4 invertedProjection = glm::inverse(camera.GetProjectionMatrix());
		glm::vec4 eyeCoords = invertedProjection * clipCoords;
		return glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);
	}

	glm::vec2 MousePicker::getNormalisedDeviceCoordinates(float mouseX, float mouseY) const
	{
		float x = (2.0f * mouseX) / (float)m_ViewportWidth - 1.0f;
		float y = (2.0f * mouseY) / (float)m_ViewportHeight - 1.0f;
		return glm::vec2(x, -y);
	}
}