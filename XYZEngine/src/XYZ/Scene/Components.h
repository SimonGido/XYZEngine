#pragma once

#include "XYZ/Core/GUID.h"

#include "XYZ/Reflection/Reflection.h"

#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/SubTexture.h"
#include "XYZ/Renderer/Mesh.h"
#include "XYZ/Renderer/StorageBufferAllocator.h"

#include "XYZ/Asset/Renderer/MaterialAsset.h"
#include "XYZ/Asset/Animation/AnimationController.h"

#include "XYZ/Script/ScriptPublicField.h"
#include "XYZ/Particle/CPU/ParticleSystem.h"
#include "XYZ/Particle/GPU/ParticleSystemGPU.h"

#include "SceneCamera.h"

#include <entt/entt.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace XYZ {

	struct XYZ_API IDComponent
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

	class XYZ_API TransformComponent
	{
	public:
		TransformComponent() = default;
		TransformComponent(const TransformComponent& other);
		TransformComponent(const glm::vec3& translation);

		struct Transform
		{
			glm::vec3 Translation = { 0.0f,0.0f,0.0f };
			glm::vec3 Rotation = { 0.0f,0.0f,0.0f };
			glm::vec3 Scale = { 1.0f,1.0f,1.0f };

			glm::mat4 WorldTransform = glm::mat4(1.0f);
		};


		const Transform * operator->() const { return &m_Transform; }

		Transform& GetTransform() { m_Dirty = true; return m_Transform; }
		
		std::tuple<glm::vec3, glm::vec3, glm::vec3> GetWorldComponents() const;

		glm::mat4 GetLocalTransform() const;
		
		void DecomposeTransform(const glm::mat4& transform);

	private:
		Transform m_Transform;
		bool	  m_Dirty = true;

		friend class Scene;
	};
	

	struct XYZ_API SceneTagComponent
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

	class XYZ_API SpriteRenderer
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

	struct XYZ_API MeshComponent
	{
		MeshComponent() = default;
		MeshComponent(const MeshComponent& other);
		MeshComponent(const Ref<StaticMesh>& mesh, const Ref<MaterialAsset>& materialAsset);

		Ref<StaticMesh>		  Mesh;
		Ref<MaterialAsset>    MaterialAsset;
		Ref<MaterialInstance> OverrideMaterial;
	};

	struct XYZ_API AnimatedMeshComponent
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

	struct XYZ_API AnimationComponent
	{
		AnimationComponent() = default;
		AnimationComponent(const AnimationComponent& other);

		Ref<AnimationController>  Controller;
		SamplingContext			  Context; // It is not owned by controller so single controller can update on multiple threads
		float					  AnimationTime = 0.0f;
		bool					  Playing = false;
	};

	class Prefab;
	struct XYZ_API PrefabComponent
	{
		PrefabComponent() = default;
		PrefabComponent(const PrefabComponent& other);
		PrefabComponent(const Ref<Prefab>& prefabAsset, const entt::entity owner);

		Ref<Prefab>	 PrefabAsset;
		entt::entity Owner = entt::null;
	};

	struct XYZ_API ParticleRenderer
	{
		ParticleRenderer() = default;
		ParticleRenderer(const ParticleRenderer& other);
		ParticleRenderer(const Ref<Mesh>&mesh, const Ref<MaterialAsset>&materialAsset);


		Ref<Mesh>		      Mesh;
		Ref<MaterialAsset>    MaterialAsset;
		Ref<MaterialInstance> OverrideMaterial;
	};

	struct XYZ_API CameraComponent
	{
		SceneCamera Camera;

		CameraComponent() = default;
		CameraComponent(const CameraComponent& other);
	};


	struct XYZ_API ParticleComponent
	{
		ParticleComponent();
		ParticleComponent(const ParticleComponent& other);

		ParticleComponent& operator =(const ParticleComponent& other);

		Ref<ParticleSystem> GetSystem() const { return m_System; }

	private:
		Ref<ParticleSystem> m_System;
	};

	struct ParticleComponentGPU
	{
		Ref<Mesh>			    Mesh;
		Ref<MaterialAsset>	    RenderMaterial;
		Ref<MaterialInstance>   OverrideMaterial;
		Ref<MaterialAsset>		UpdateMaterial;
		Ref<ParticleSystemGPU>  System;
	};

	struct XYZ_API PointLightComponent2D
	{
		PointLightComponent2D() = default;
		PointLightComponent2D(const glm::vec3& color, float radius, float intensity);
		PointLightComponent2D(const PointLightComponent2D& other);

		glm::vec3 Color = glm::vec3(1.0f);
		float	  Radius	= 1.0f;
		float	  Intensity = 1.0f;
	};
	

	struct XYZ_API SpotLightComponent2D
	{
		SpotLightComponent2D() = default;
		SpotLightComponent2D(const SpotLightComponent2D& other);

		glm::vec3 Color  = glm::vec3(1.0f);
		float Radius	 = 1.0f;
		float Intensity  = 1.0f;
		float InnerAngle = -180.0f;
		float OuterAngle =  180.0f;
	};


	struct PointLightComponent3D
	{
		glm::vec3 Radiance = { 1.0f, 1.0f, 1.0f };
		float	  Intensity = 1.0f;
		float	  LightSize = 0.5f; // For PCSS
		float	  MinRadius = 1.f;
		float	  Radius = 10.f;
		bool	  CastsShadows = true;
		bool	  SoftShadows = true;
		float	  Falloff = 1.f;
	};

	struct XYZ_API Relationship
	{
		Relationship();
		Relationship(const Relationship& other);

		template <typename T>
		entt::entity Find(const entt::registry& reg, const T& component) const;
		entt::entity FindByName(const entt::registry& reg, std::string_view name) const;

		std::vector<entt::entity> GetTree(const entt::registry& reg) const;
		bool					  IsInHierarchy(const entt::registry& reg, entt::entity child) const;

		entt::entity GetParent()		  const { return Parent; }
		entt::entity GetFirstChild()	  const { return FirstChild; }
		entt::entity GetPreviousSibling() const { return PreviousSibling; }
		entt::entity GetNextSibling()	  const { return NextSibling; }
		uint32_t	 GetDepth()			  const { return Depth; }

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
		return entt::entity{};
	}

	struct XYZ_API ScriptComponent
	{
		std::string ModuleName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent& other)
			: ModuleName(other.ModuleName) {}
		ScriptComponent(const std::string & moduleName)
			: ModuleName(moduleName) {}

	};


	struct XYZ_API RigidBody2DComponent
	{
		enum class BodyType { Static, Dynamic, Kinematic };

		BodyType Type;

		void* RuntimeBody = nullptr;
	};


	struct XYZ_API BoxCollider2DComponent
	{
		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent& other);

		glm::vec2 Size = glm::vec2(1.0f);
		glm::vec2 Offset = glm::vec2(0.0f);
		float Density  = 1.0f;
		float Friction = 0.0f;

		void* RuntimeFixture = nullptr;
	};


	struct XYZ_API CircleCollider2DComponent
	{
		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent& other);

		glm::vec2 Offset = glm::vec2(0.0f);
		float	  Radius = 1.0f;
		float	  Density = 1.0f;
		float	  Friction = 0.0f;
				  
		void*	  RuntimeFixture = nullptr;
	};


	struct XYZ_API PolygonCollider2DComponent
	{
		PolygonCollider2DComponent() = default;
		PolygonCollider2DComponent(const PolygonCollider2DComponent& other);

		std::vector<glm::vec2> Vertices;

		float Density = 1.0f;
		float Friction = 0.0f;

		void* RuntimeFixture = nullptr;
	};
	
	struct XYZ_API ChainCollider2DComponent
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
		bool  InvertNormals = false;
		void* RuntimeFixture = nullptr;
	};


	template <typename T>
	class XYZ_API Replicable
	{
	public:
		Replicable(T& data)
			: m_Data(data)
		{}
		const T* operator->() const { return &m_Data; }

		T& Replicate() { m_Dirty = true; return m_Data; }

		const T& operator() const { return m_Data; }
	private:
		T& m_Data;
		static entt::id_type s_ID;
	};

	template <typename T>
	entt::id_type Replicable<T>::s_ID = entt::type_hash<T>::value();

#define XYZ_COMPONENTS \
	IDComponent, \
	SceneTagComponent, \
	TransformComponent, \
	SpriteRenderer, \
	MeshComponent, \
	AnimatedMeshComponent, \
	AnimationComponent, \
	PrefabComponent, \
	ParticleRenderer, \
	CameraComponent,\
	ParticleComponent, \
	ParticleComponentGPU, \
	PointLightComponent2D, \
	SpotLightComponent2D, \
	PointLightComponent3D, \
	Relationship, \
	ScriptComponent, \
	RigidBody2DComponent, \
	BoxCollider2DComponent, \
	CircleCollider2DComponent, \
	PolygonCollider2DComponent, \
	ChainCollider2DComponent
}