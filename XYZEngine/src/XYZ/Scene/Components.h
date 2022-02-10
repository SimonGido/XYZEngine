#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "XYZ/ECS/Component.h"
#include "XYZ/Core/GUID.h"

#include "XYZ/Particle/GPU/ParticleMaterial.h"
#include "XYZ/Particle/GPU/ParticleSystem.h"
#include "XYZ/Particle/CPU/ParticleSystemCPU.h"

#include "XYZ/Reflection/Reflection.h"

#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Script/ScriptPublicField.h"
#include "SceneCamera.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace XYZ {

	struct IDComponent 
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
	class TransformComponent 
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
		
		void DecomposeTransform(const glm::mat4& transform);
	};
	REFLECTABLE(TransformComponent, Translation, Rotation, Scale, WorldTransform);

	struct SceneTagComponent 
	{
		std::string Name;
		SceneTagComponent() = default;
		SceneTagComponent(const std::string& name)
			: Name(name)
		{}
		SceneTagComponent(const SceneTagComponent& other)
			: Name(other.Name)
		{}
		SceneTagComponent(std::string&& name)
			: Name(std::move(name))
		{}
		bool operator==(const SceneTagComponent& other) const
		{
			return Name == other.Name;
		}

		operator std::string& () { return Name; }
		operator const  std::string& () const { return Name; }
	};

	struct SpriteRenderer 
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
	REFLECTABLE(SpriteRenderer, Material, SubTexture, Color, SortLayer, Visible);


	class Mesh;
	struct MeshComponent
	{
		Ref<Mesh> Mesh;
	};

	struct CameraComponent 
	{
		SceneCamera Camera;
		CameraComponent() = default;
	};


	class Animator;
	class AnimatorController;
	struct AnimatorComponent 
	{
		AnimatorComponent() = default;
		Ref<Animator>			Animator;
		Ref<AnimatorController> Controller;
	};


	struct ParticleComponentGPU 
	{
		ParticleComponentGPU() = default;
		
		Ref<ParticleSystem>    System;
	};

	struct ParticleComponentCPU 
	{
		ParticleComponentCPU() = default;
		
		ParticleSystemCPU System;
	};


	struct PointLight2D 
	{
		PointLight2D() = default;

		glm::vec3 Color = glm::vec3(1.0f);
		float	  Radius	= 1.0f;
		float	  Intensity = 1.0f;
	};
	REFLECTABLE(PointLight2D, Color, Radius, Intensity);


	struct SpotLight2D 
	{
		glm::vec3 Color  = glm::vec3(1.0f);
		float Radius	 = 1.0f;
		float Intensity  = 1.0f;
		float InnerAngle = -180.0f;
		float OuterAngle =  180.0f;
	};
	REFLECTABLE(SpotLight2D, Color, Radius, Intensity, InnerAngle, OuterAngle);


	struct Relationship 
	{
		Relationship();
		Relationship(Entity parent);
		
		template <typename T>
		Entity Find(const ECSManager& ecs, const T& component) const;
		Entity FindByName(const ECSManager& ecs, std::string_view name) const;

		std::vector<Entity> GetTree(const ECSManager& ecs) const;
		std::string			GetPath(const ECSManager& ecs, Entity entity, Entity end = Entity(0)) const;
		bool			    IsInHierarchy(const ECSManager& ecs, Entity parent, std::string_view path) const;
		bool				IsInHierarchy(const ECSManager& ecs, Entity child) const;

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

	template<typename T>
	inline Entity Relationship::Find(const ECSManager& ecs, const T& component) const
	{
		std::stack<Entity> temp;
		temp.push(FirstChild);
		while (!temp.empty())
		{
			Entity entity = temp.top();
			temp.pop();
			if (ecs.Contains<T>(entity))
			{
				if (ecs.GetComponent<T>(entity) == component)
					return entity;
			}
			const auto& relationship = ecs.GetComponent<Relationship>(entity);
			if (relationship.NextSibling)
				temp.push(relationship.NextSibling);
			if (relationship.FirstChild)
				temp.push(relationship.FirstChild);
		}
		return Entity();
	}

	struct ScriptComponent 
	{
		std::string ModuleName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent & other) = default;
		ScriptComponent(const std::string & moduleName)
			: ModuleName(moduleName) {}

	};


	struct RigidBody2DComponent 
	{
		enum class BodyType { Static, Dynamic, Kinematic };

		BodyType Type;

		void* RuntimeBody = nullptr;
	};


	struct BoxCollider2DComponent 
	{
		glm::vec2 Size = glm::vec2(1.0f);
		glm::vec2 Offset = glm::vec2(0.0f);
		float Density  = 1.0f;
		float Friction = 0.0f;

		void* RuntimeFixture = nullptr;
	};
	REFLECTABLE(BoxCollider2DComponent, Size, Offset, Density, Friction, RuntimeFixture);


	struct CircleCollider2DComponent 
	{
		glm::vec2 Offset = glm::vec2(0.0f);
		float	  Radius = 1.0f;
		float	  Density = 1.0f;
		float	  Friction = 0.0f;
				  
		void*	  RuntimeFixture = nullptr;
	};
	REFLECTABLE(CircleCollider2DComponent, Offset, Radius, Density, Friction, RuntimeFixture);

	struct PolygonCollider2DComponent 
	{
		std::vector<glm::vec2> Vertices;

		float Density = 1.0f;
		float Friction = 0.0f;

		void* RuntimeFixture = nullptr;
	};
	
	struct ChainCollider2DComponent 
	{
		ChainCollider2DComponent()
		{
			Points.push_back(glm::vec2(0.0f));
			Points.push_back(glm::vec2(0.0f));
		}
		std::vector<glm::vec2> Points;

		float Density = 1.0f;
		float Friction = 0.0f;

		void* RuntimeFixture = nullptr;
	};

	REGISTER_REFLECTABLES(TransformComponent, SpriteRenderer, PointLight2D, SpotLight2D, BoxCollider2DComponent, CircleCollider2DComponent);
	

}