#pragma once
#include "XYZ/ECS/ECSManager.h"

#include <glm/glm.hpp>

namespace XYZ {

	class Transform : public Type<Transform>
	{
	public:
		Transform(const glm::vec3& pos, const glm::vec2& scale = glm::vec2(1), float rot = 0.0f);

		void Translate(const glm::vec3& translation);
		void Scale(const glm::vec2& scale);
		void Rotate(float rotation);

		void SetTranslation(const glm::vec3& translation);
		void SetScale(const glm::vec2& scale);
		void SetRotation(float rotation);

		void SetParent(Transform* parent);
		void CalculateWorldTransformation();

		void DetachParent();

		// Does not set m_Updated to true
		const glm::vec3& GetPosition() const { return m_Position; };
		const glm::vec2& GetScale() const { return m_Scale; };
		float GetRotation() const { return m_Rotation; };

		const glm::vec3 GetWorldPosition() const;
		const glm::vec2 GetWorldScale() const;

		const glm::mat4& GetTransformation() const;

		bool Updated() const { return m_Updated; }
	private:
		glm::mat4 calculateTransform() const;

	private:
		glm::mat4 m_Transformation = glm::mat4(1);

		glm::vec3 m_Position;
		glm::vec2 m_Scale;
		float m_Rotation;


		Transform* m_Parent = nullptr;

		// It is required to be mutable , we use GetTransformation in functions 
		// that are not allowed to modify passed references
		mutable bool m_Updated = true;
	};



}