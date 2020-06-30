#pragma once
#include "XYZ/ECS/ECSManager.h"

#include <glm/glm.hpp>

namespace XYZ {

	class Transform2D : public Type<Transform2D>
	{
	public:
		Transform2D(const glm::vec3& pos, const glm::vec2& scale = glm::vec2(1), float rot = 0.0f);

		void Translate(const glm::vec3& translation);
		void Scale(const glm::vec2& scale);
		void Rotate(float rotation);

		void SetTranslation(const glm::vec3& translation);
		void SetScale(const glm::vec2& scale);
		void SetRotation(float rotation);

		void InheritParent(const Transform2D& parent);
		void SetParent(Transform2D* parent) { m_Parent = parent; };
		void CalculateWorldTransformation();

		void DetachParent();

		// Does not set m_Updated to true
		const glm::vec3& GetPosition() const { return m_Position; };
		const glm::vec2& GetScale() const { return m_Scale; };
		float GetRotation() const { return m_Rotation; };



		const glm::mat4& GetTransformation() const;

		bool Updated() const { return m_Updated; }
	private:
		glm::mat4 calculateTransform() const;

	private:
		glm::vec3 m_Position;
		glm::vec2 m_Scale;
		float m_Rotation;

		// It is required to be mutable , we use GetTransformation in functions 
		// that are not allowed to modify passed references
		mutable glm::mat4 m_Transformation = glm::mat4(1);

		Transform2D* m_Parent = nullptr;
		// It is required to be mutable , we use GetTransformation in functions 
		// that are not allowed to modify passed references
		mutable bool m_Updated = true;
	};



}