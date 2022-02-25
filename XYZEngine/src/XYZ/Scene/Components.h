#pragma once

#include "XYZ/Core/GUID.h"

#include "XYZ/Reflection/Reflection.h"

#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/Mesh.h"
#include "XYZ/Asset/Renderer/MaterialAsset.h"

#include "XYZ/Script/ScriptPublicField.h"
#include "XYZ/Particle/CPU/ParticleSystem.h"

#include "SceneCamera.h"

#include <entt/entt.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace XYZ {

	struct IDComponent 
	{
		IDComponent() = default;
		IDComponent(const IDComponent& other);
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
		TransformComponent(const TransformComponent& other);
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

	class SpriteRenderer 
	{
	public:
		SpriteRenderer() = default;
		SpriteRenderer(
			const Ref<MaterialAsset>& material,
			const Ref<SubTexture>& subTexture,
			const glm::vec4& color,
			uint32_t sortLayer,
			bool visible = true
		);

		SpriteRenderer(const SpriteRenderer& other);
		SpriteRenderer(SpriteRenderer&& other) noexcept;


		SpriteRenderer& operator =(const SpriteRenderer& other);

		Ref<MaterialAsset> Material;
		Ref<SubTexture>	   SubTexture;
		
		glm::vec4 Color = glm::vec4(1.0f);
		uint32_t  SortLayer = 0;
		bool	  Visible = true;
	};

	struct MeshComponent
	{
		MeshComponent() = default;
		MeshComponent(const MeshComponent& other);
		MeshComponent(const Ref<Mesh>& mesh, const Ref<MaterialAsset>& materialAsset);

		Ref<Mesh>		      Mesh;
		Ref<MaterialAsset>    MaterialAsset;
		Ref<MaterialInstance> OverrideMaterial;
	};

	struct AnimatedMeshComponent
	{
		AnimatedMeshComponent() = default;
		AnimatedMeshComponent(const AnimatedMeshComponent& other);
		AnimatedMeshComponent(const Ref<AnimatedMesh>& mesh, const Ref<MaterialAsset>& materialAsset);

		Ref<AnimatedMesh>	  Mesh;
		Ref<MaterialAsset>    MaterialAsset;
		Ref<MaterialInstance> OverrideMaterial;
		
		std::vector<ozz::math::Float4x4> BoneTransforms;
		std::vector<entt::entity>		 BoneEntities;
	};

	class AnimationController;
	struct AnimationComponent
	{
		AnimationComponent() = default;
		AnimationComponent(const AnimationComponent& other);

		Ref<AnimationController>  Controller;
		std::vector<entt::entity> BoneEntities; 
		float					  AnimationTime = 0.0f;
		bool					  Playing = false;
	};

	class Prefab;
	struct PrefabComponent
	{
		PrefabComponent() = default;
		PrefabComponent(const PrefabComponent& other);
		PrefabComponent(const Ref<Prefab>& prefabAsset, const entt::entity owner);

		Ref<Prefab>	 PrefabAsset;
		entt::entity Owner = entt::null;
	};

	struct ParticleRenderer
	{
		ParticleRenderer() = default;
		ParticleRenderer(const ParticleRenderer& other);
		ParticleRenderer(const Ref<Mesh>&mesh, const Ref<MaterialAsset>&materialAsset);


		Ref<Mesh>		      Mesh;
		Ref<MaterialAsset>    MaterialAsset;
		Ref<MaterialInstance> OverrideMaterial;
	};

	struct CameraComponent 
	{
		SceneCamera Camera;

		CameraComponent() = default;
		CameraComponent(const CameraComponent& other);
	};


	struct ParticleComponent
	{
		ParticleComponent() = default;
		ParticleComponent(const ParticleComponent& other);

		ParticleSystem System;
	};


	struct PointLight2D 
	{
		PointLight2D() = default;
		PointLight2D(const glm::vec3& color, float radius, float intensity);
		PointLight2D(const PointLight2D& other);

		glm::vec3 Color = glm::vec3(1.0f);
		float	  Radius	= 1.0f;
		float	  Intensity = 1.0f;
	};
	

	struct SpotLight2D 
	{
		SpotLight2D() = default;
		SpotLight2D(const SpotLight2D& other);

		glm::vec3 Color  = glm::vec3(1.0f);
		float Radius	 = 1.0f;
		float Intensity  = 1.0f;
		float InnerAngle = -180.0f;
		float OuterAngle =  180.0f;
	};


	struct Relationship 
	{
		Relationship();
		Relationship(const Relationship& other);

		template <typename T>
		entt::entity Find(const entt::registry& reg, const T& component) const;
		entt::entity FindByName(const entt::registry& reg, std::string_view name) const;

		std::vector<entt::entity> GetTree(const entt::registry& reg) const;
		bool					  IsInHierarchy(const entt::registry& reg, entt::entity child) const;

		entt::entity GetParent() const { return Parent; }
		entt::entity GetFirstChild() const { return FirstChild; }
		entt::entity GetPreviousSibling() const { return PreviousSibling; }
		entt::entity GetNextSibling() const { return NextSibling; }
		uint32_t GetDepth() const { return Depth; }

		static void SetupRelation(entt::entity parent, entt::entity child, entt::registry& reg);
		static void RemoveRelation(entt::entity child, entt::registry& reg);

	private:
		static void removeRelation(entt::entity child, entt::registry& reg);

	private:
		entt::entity Parent;
		entt::entity FirstChild;
		entt::entity PreviousSibling;
		entt::entity NextSibling;

		uint32_t Depth;
		friend class Scene;
		friend class SceneSerializer;
	};

	template<typename T>
	inline entt::entity Relationship::Find(const entt::registry& reg, const T& component) const
	{
		std::stack<entt::entity> temp;
		temp.push(FirstChild);
		while (!temp.empty())
		{
			entt::entity entity = temp.top();
			temp.pop();
			if (reg.any_of<T>(entity))
			{
				if (reg.get<T>(entity) == component)
					return entity;
			}
			const auto& relationship = reg.get<Relationship>(entity);
			if (reg.valid(relationship.NextSibling))
				temp.push(relationship.NextSibling);
			if (reg.valid(relationship.FirstChild))
				temp.push(relationship.FirstChild);
		}
		return Entity();
	}

	struct ScriptComponent 
	{
		std::string ModuleName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent& other)
			: ModuleName(other.ModuleName) {}
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
		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent& other);

		glm::vec2 Size = glm::vec2(1.0f);
		glm::vec2 Offset = glm::vec2(0.0f);
		float Density  = 1.0f;
		float Friction = 0.0f;

		void* RuntimeFixture = nullptr;
	};


	struct CircleCollider2DComponent 
	{
		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent& other);

		glm::vec2 Offset = glm::vec2(0.0f);
		float	  Radius = 1.0f;
		float	  Density = 1.0f;
		float	  Friction = 0.0f;
				  
		void*	  RuntimeFixture = nullptr;
	};


	struct PolygonCollider2DComponent 
	{
		PolygonCollider2DComponent() = default;
		PolygonCollider2DComponent(const PolygonCollider2DComponent& other);

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
		ChainCollider2DComponent(const ChainCollider2DComponent& other);

		std::vector<glm::vec2> Points;

		float Density = 1.0f;
		float Friction = 0.0f;

		void* RuntimeFixture = nullptr;
	};
}