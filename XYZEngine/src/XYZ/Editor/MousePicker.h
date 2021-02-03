#pragma once

#include "XYZ/Core/Input.h"
#include "XYZ/Editor/EditorCamera.h"
#include "XYZ/Scene/Scene.h"
#include "XYZ/Scene/SceneEntity.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>


namespace XYZ {

	class MousePicker
	{
	public:
		void Update(const EditorCamera& camera);
		void SetViewportSize(uint32_t width, uint32_t height) { m_ViewportWidth = width; m_ViewportHeight = height; }
		const glm::vec3& GetRay() const { return m_CurrentRay; }

	private:
		glm::vec3 calculateMouseRay(const EditorCamera& camera) const;
		glm::vec3 toWorldCoords(const EditorCamera& camera, const glm::vec4& eyeCoords) const;
		glm::vec4 toEyeCoords(const EditorCamera& camera, const glm::vec4& clipCoords) const;
		glm::vec2 getNormalisedDeviceCoordinates(float mouseX, float mouseY) const;
		
	private:
		glm::vec3 m_CurrentRay = glm::vec3(1.0f);

		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;
	};

}