#pragma once
#include "XYZ/EntityComponentSystem/Component.h"
#include "XYZ/Particle/ParticleEffect.h"
#include "XYZ/Renderer/SubTexture2D.h"

#include "SceneCamera.h"
#include "AnimationController.h"


#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace XYZ {
	class ScriptableEntity;

	class TransformComponent : public ECS::Type<TransformComponent>
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
	};

	struct SceneTagComponent : public ECS::Type<SceneTagComponent>
	{
		std::string Name;
		SceneTagComponent(const std::string& name)
			: Name(name)
		{}
		SceneTagComponent(const SceneTagComponent& other)
			: Name(other.Name)
		{}

		operator std::string& () { return Name; }
		operator const  std::string& () const { return Name; }
	};

	struct SpriteRenderer : public ECS::Type<SpriteRenderer>
	{
		SpriteRenderer() = default;

		SpriteRenderer(
			Ref<Material> material,
			Ref<SubTexture2D> subTexture,
			const glm::vec4& color,
			uint32_t textureID,
			int32_t sortLayer,
			bool isVisible = true
		);

		SpriteRenderer(const SpriteRenderer& other);
		SpriteRenderer(SpriteRenderer&& other) noexcept;


		SpriteRenderer& operator =(const SpriteRenderer& other);

		Ref<Material> Material;
		Ref<SubTexture2D> SubTexture;
		glm::vec4 Color;

		uint32_t TextureID;
		int32_t SortLayer = 0;
		bool IsVisible = true;
	};


	struct CameraComponent : public ECS::Type<CameraComponent>
	{
		SceneCamera Camera;
		CameraComponent() = default;
	};


	struct NativeScriptComponent : public ECS::Type<NativeScriptComponent>
	{
		NativeScriptComponent() = default;
		NativeScriptComponent(ScriptableEntity* scriptableEntity, const std::string& scriptObjectName)
			:
			ScriptableEntity(scriptableEntity),
			ScriptObjectName(scriptObjectName)
		{}

		ScriptableEntity* ScriptableEntity = nullptr;
		std::string ScriptObjectName;
	};

	struct AnimatorComponent : public ECS::Type<AnimatorComponent>
	{
		AnimatorComponent() = default;
		
		Ref<AnimationController> Controller;
	};



	struct ParticleComponent : public ECS::Type<ParticleComponent>
	{
		ParticleComponent() = default;
		Ref<MaterialInstance> RenderMaterial;
		Ref<MaterialInstance> ComputeMaterial;
		Ref<ParticleEffect> ParticleEffect;

		uint32_t TextureID = 0;
	};

	struct PointLight2D : public ECS::Type<PointLight2D>
	{
		PointLight2D() = default;

		glm::vec3 Color = glm::vec3(1.0f);
		float Intensity = 1.0f;
	};
}