#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/ECS/Component.h"
#include "XYZ/Core/GUID.h"
#include "XYZ/Particle/ParticleEffect.h"
#include "XYZ/Renderer/SubTexture.h"

#include "XYZ/Script/ScriptPublicField.h"
#include "XYZ/Utils/Math/AABB.h"
#include "XYZ/Physics/PhysicsBody.h"

#include "SceneCamera.h"

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
		
		glm::mat4 WorldTransform = glm::mat4(1.0f);

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
	};

	struct SpriteRenderer : public IComponent
	{
		SpriteRenderer() = default;
		SpriteRenderer(
			const Ref<Material>& material,
			const Ref<SubTexture>& subTexture,
			const glm::vec4& color,
			uint32_t sortLayer,
			bool isVisible = true
		);

		SpriteRenderer(const SpriteRenderer& other);
		SpriteRenderer(SpriteRenderer&& other) noexcept;


		SpriteRenderer& operator =(const SpriteRenderer& other);

		Ref<Material>	Material;
		Ref<SubTexture> SubTexture;
		
		glm::vec4 Color = glm::vec4(1.0f);
		uint32_t  SortLayer = 0;
		bool	  IsVisible = true;
	};



	struct CameraComponent : public IComponent
	{
		SceneCamera Camera;
		CameraComponent() = default;
	};


	class Animation;
	struct AnimatorComponent : public IComponent
	{
		AnimatorComponent() = default;
		Ref<Animation> Animation;
	};


	struct ParticleComponent : public IComponent
	{
		ParticleComponent() = default;
		Ref<MaterialInstance> RenderMaterial;
		Ref<MaterialInstance> ComputeMaterial;
		Ref<ParticleEffect> ParticleEffect;

		uint32_t TextureID = 0;
	};

	struct PointLight2D : public IComponent
	{
		PointLight2D() = default;

		glm::vec3 Color = glm::vec3(1.0f);
		float Intensity = 1.0f;
	};


	struct Relationship : public IComponent
	{
		Relationship() = default;
		Relationship(Entity parent);

		Entity GetParent() const { return Parent; }
		Entity GetFirstChild() const { return FirstChild; }
		Entity GetPreviousSibling() const { return PreviousSibling; }
		Entity GetNextSibling() const { return NextSibling; }
		
		static void SetupRelation(Entity parent, Entity child, ECSManager& ecs);
		static void RemoveRelation(Entity child, ECSManager& ecs);
		
		static std::function<void(Entity entity, ECSManager& ecs)> OnParentChanged;
	private:
		static void removeRelation(Entity child, ECSManager& ecs);

	private:
		Entity Parent;
		Entity FirstChild;
		Entity PreviousSibling;
		Entity NextSibling;

		friend class Scene;
		friend class SceneSerializer;
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
	};


	struct BoxCollider2DComponent : public IComponent
	{
		PhysicsShape* Shape = nullptr;

		glm::vec2 Offset = glm::vec2(0.0f);
		glm::vec2 Size = glm::vec2(1.0f);
		float Density = 1.0f;
	};

	struct CircleCollider2DComponent : public IComponent
	{
		PhysicsShape* Shape = nullptr;

		glm::vec2 Offset = glm::vec2(0.0f);
		float Radius = 1.0f;
		float Density = 1.0f;
	};
}