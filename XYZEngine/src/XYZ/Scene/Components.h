#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/ECS/Component.h"
#include "XYZ/ECS/Types.h"
#include "XYZ/Core/GUID.h"
#include "XYZ/Particle/ParticleEffect.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Script/ScriptPublicField.h"
#include "XYZ/Utils/Math/AABB.h"
#include "XYZ/Physics/PhysicsBody.h"

#include "SceneCamera.h"
#include "AnimationController.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace XYZ {

	struct IDComponent : public IComponent
	{
		IDComponent() = default;
		IDComponent(const GUID& id) 
			: ID(id)
		{}
		bool operator==(const IDComponent& other) const
		{
			return (std::string)ID == (std::string)other.ID;
		}

		virtual void Copy(IComponent* component) override
		{
			ID = static_cast<IDComponent*>(component)->ID;
		}
		GUID ID;
	};

	class TransformComponent : public IComponent
	{
	public:
		TransformComponent() = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation)
		{}
		
		glm::vec3 Translation = { 0.0f,0.0f,0.0f };
		glm::vec3 Rotation = { 0.0f,0.0f,0.0f };
		glm::vec3 Scale = { 1.0f,1.0f,1.0f };
		
		glm::mat4 GetTransform() const
		{		
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), Rotation.x, { 1, 0, 0 })
				* glm::rotate(glm::mat4(1.0f), Rotation.y, { 0, 1, 0 })
				* glm::rotate(glm::mat4(1.0f), Rotation.z, { 0, 0, 1 });


			return glm::translate(glm::mat4(1.0f), Translation)
					* rotation
					* glm::scale(glm::mat4(1.0f), Scale);
		}


		void DecomposeTransform(const glm::mat4& transform)
		{
			glm::quat rotation;
			glm::vec3 skew;
			glm::vec4 perspective;
				
			glm::decompose(transform, Scale, rotation, Translation, skew, perspective);
			Rotation = glm::eulerAngles(rotation) * glm::pi<float>() / 180.f;
		}

		virtual void Copy(IComponent* component) override
		{
			TransformComponent* casted = static_cast<TransformComponent*>(component);
			Translation = casted->Translation;
			Rotation = casted->Rotation;
			Scale = casted->Scale;
		}
	};

	struct SceneTagComponent : public IComponent
	{
		std::string Name;
		SceneTagComponent() = default;
		SceneTagComponent(const std::string& name)
			: Name(name)
		{}
		SceneTagComponent(const SceneTagComponent& other)
			: Name(other.Name)
		{}

		bool operator==(const SceneTagComponent& other) const
		{
			return Name == other.Name;
		}

		operator std::string& () { return Name; }
		operator const  std::string& () const { return Name; }

		virtual void Copy(IComponent* component) override
		{
			Name = static_cast<SceneTagComponent*>(component)->Name;
		}
	};

	struct SpriteRenderer : public IComponent
	{
		SpriteRenderer() = default;
		SpriteRenderer(
			Ref<Material> material,
			Ref<SubTexture> subTexture,
			const glm::vec4& color,
			uint32_t sortLayer,
			bool isVisible = true
		);

		SpriteRenderer(const SpriteRenderer& other);
		SpriteRenderer(SpriteRenderer&& other) noexcept;


		SpriteRenderer& operator =(const SpriteRenderer& other);

		Ref<Material> Material;
		Ref<SubTexture> SubTexture;
		glm::vec4 Color;

		uint32_t SortLayer = 0;
		bool IsVisible = true;

		virtual void Copy(IComponent* component) override
		{
			SpriteRenderer* casted = static_cast<SpriteRenderer*>(component);
			Material = casted->Material;
			SubTexture = casted->SubTexture;
			Color = casted->Color;
			IsVisible = casted->IsVisible;
		}
	};


	struct CameraComponent : public IComponent
	{
		SceneCamera Camera;
		CameraComponent() = default;

		virtual void Copy(IComponent* component) override
		{
			Camera = static_cast<CameraComponent*>(component)->Camera;
		}
	};


	struct AnimatorComponent : public IComponent
	{
		AnimatorComponent() = default;
		AnimationController Controller;

		virtual void Copy(IComponent* component) override
		{
			Controller = static_cast<AnimatorComponent*>(component)->Controller;
		}
	};


	struct ParticleComponent : public IComponent
	{
		ParticleComponent() = default;
		Ref<MaterialInstance> RenderMaterial;
		Ref<MaterialInstance> ComputeMaterial;
		Ref<ParticleEffect> ParticleEffect;

		uint32_t TextureID = 0;

		virtual void Copy(IComponent* component) override
		{
			ParticleComponent* casted = static_cast<ParticleComponent*>(component);
			RenderMaterial = casted->RenderMaterial;
			ComputeMaterial = casted->ComputeMaterial;
			ParticleEffect = casted->ParticleEffect;
		}
	};

	struct PointLight2D : public IComponent
	{
		PointLight2D() = default;

		glm::vec3 Color = glm::vec3(1.0f);
		float Intensity = 1.0f;

		virtual void Copy(IComponent* component) override
		{
			PointLight2D* casted = static_cast<PointLight2D*>(component);
			
			Color = casted->Color;
			Intensity = casted->Intensity;
		}
	};


	struct Relationship : public IComponent
	{
		uint32_t Parent			 = NULL_ENTITY;
		uint32_t FirstChild		 = NULL_ENTITY;
		uint32_t PreviousSibling = NULL_ENTITY;
		uint32_t NextSibling	 = NULL_ENTITY;

		static void SetupRelation(uint32_t parent, uint32_t child, ECSManager& ecs);
		static void RemoveRelation(uint32_t child, ECSManager& ecs);

		virtual void Copy(IComponent* component) override
		{
			Relationship* casted = static_cast<Relationship*>(component);
			Parent = casted->Parent;
			FirstChild = casted->FirstChild;
			PreviousSibling = casted->PreviousSibling;
			NextSibling = casted->NextSibling;
		}
	};

	struct EntityScriptClass;
	struct ScriptComponent : public IComponent
	{
		std::string ModuleName;
		std::vector<PublicField> Fields;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent & other) = default;
		ScriptComponent(const std::string & moduleName)
			: ModuleName(moduleName) {}


		virtual void Copy(IComponent* component) override
		{
			ScriptComponent* casted = static_cast<ScriptComponent*>(component);
			ModuleName = casted->ModuleName;
			Fields = casted->Fields;
			ScriptClass = casted->ScriptClass;
			Handle = casted->Handle;
		}
	private:
		EntityScriptClass* ScriptClass = nullptr;
		
		uint32_t Handle = 0;

		friend class ScriptEngine;
	};


	struct RigidBody2DComponent : public IComponent
	{
		enum class BodyType { Static, Dynamic, Kinematic };

		BodyType Type;

		PhysicsBody* Body = nullptr;

		virtual void Copy(IComponent* component) override
		{
			RigidBody2DComponent* casted = static_cast<RigidBody2DComponent*>(component);
			Type = casted->Type;
			Body = casted->Body;
		}
	};


	struct BoxCollider2DComponent : public IComponent
	{
		PhysicsShape* Shape = nullptr;

		glm::vec2 Offset = glm::vec2(0.0f);
		glm::vec2 Size = glm::vec2(1.0f);
		float Density = 1.0f;


		virtual void Copy(IComponent* component) override
		{
			BoxCollider2DComponent* casted = static_cast<BoxCollider2DComponent*>(component);
			Shape = casted->Shape;
			Offset = casted->Offset;
			Size = casted->Size;
			Density = casted->Density;
		}
	};

	struct CircleCollider2DComponent : public IComponent
	{
		PhysicsShape* Shape = nullptr;

		glm::vec2 Offset = glm::vec2(0.0f);
		float Radius = 1.0f;
		float Density = 1.0f;


		virtual void Copy(IComponent* component) override
		{
			CircleCollider2DComponent* casted = static_cast<CircleCollider2DComponent*>(component);
			Shape = casted->Shape;
			Offset = casted->Offset;
			Radius = casted->Radius;
			Density = casted->Density;
		}
	};
}