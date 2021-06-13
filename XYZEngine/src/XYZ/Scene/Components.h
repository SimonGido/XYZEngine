#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/ECS/Component.h"
#include "XYZ/Core/GUID.h"
#include "XYZ/Particle/ParticleSystem.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Script/ScriptPublicField.h"
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

		std::tuple<glm::vec3, glm::vec3, glm::vec3> GetWorldComponents() const;

		glm::mat4 GetTransform() const;
		


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
			bool visible = true
		);

		SpriteRenderer(const SpriteRenderer& other);
		SpriteRenderer(SpriteRenderer&& other) noexcept;


		SpriteRenderer& operator =(const SpriteRenderer& other);

		Ref<Material>	Material;
		Ref<SubTexture> SubTexture;
		
		glm::vec4 Color = glm::vec4(1.0f);
		uint32_t  SortLayer = 0;
		bool	  Visible = true;
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
		
		Ref<Material>		  RenderMaterial;
		Ref<Shader>			  ComputeShader;
		Ref<ParticleSystem>	  System;
	};

	struct PointLight2D : public IComponent
	{
		PointLight2D() = default;

		glm::vec3 Color = glm::vec3(1.0f);
		float Radius = 1.0f;
		float Intensity = 1.0f;
	};


	struct Relationship : public IComponent
	{
		Relationship();
		Relationship(Entity parent);

		Entity GetParent() const { return Parent; }
		Entity GetFirstChild() const { return FirstChild; }
		Entity GetPreviousSibling() const { return PreviousSibling; }
		Entity GetNextSibling() const { return NextSibling; }
		uint32_t GetDepth() const { return Depth; }

		static void SetupRelation(Entity parent, Entity child, ECSManager& ecs);
		static void RemoveRelation(Entity child, ECSManager& ecs);

	private:
		static void removeRelation(Entity child, ECSManager& ecs);

	private:
		Entity Parent;
		Entity FirstChild;
		Entity PreviousSibling;
		Entity NextSibling;

		uint32_t Depth;
		friend class Scene;
		friend class SceneSerializer;
	};

	struct EntityScriptClass;
	struct ScriptComponent : public IComponent
	{
		std::string ModuleName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent & other) = default;
		ScriptComponent(const std::string & moduleName)
			: ModuleName(moduleName) {}

		const std::vector<PublicField>& GetFields() const { return Fields; }

	private:
		EntityScriptClass* ScriptClass = nullptr;
		std::vector<PublicField> Fields;

		friend class ScriptEngine;
	};


	struct RigidBody2DComponent : public IComponent
	{
		enum class BodyType { Static, Dynamic, Kinematic };

		BodyType Type;

		void* RuntimeBody = nullptr;
	};


	struct BoxCollider2DComponent : public IComponent
	{
		glm::vec2 Size = glm::vec2(1.0f);
		float Density  = 1.0f;
		float Friction = 0.0f;

		void* RuntimeFixture = nullptr;
	};

	struct CircleCollider2DComponent : public IComponent
	{
		float Radius = 1.0f;
		float Density = 1.0f;
		float Friction = 0.0f;

		void* RuntimeFixture = nullptr;
	};

	struct PolygonCollider2DComponent : public IComponent
	{
		std::vector<glm::vec2> Vertices;

		float Density = 1.0f;
		float Friction = 0.0f;

		void* RuntimeFixture = nullptr;
	};

	struct ChainCollider2DComponent : public IComponent
	{
		std::vector<glm::vec2> Points;

		float Density = 1.0f;
		float Friction = 0.0f;

		void* RuntimeFixture = nullptr;
	};
}