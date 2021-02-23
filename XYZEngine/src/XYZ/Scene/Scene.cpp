#include "stdafx.h"
#include "Scene.h"

#include "XYZ/Core/Input.h"
#include "XYZ/Core/Application.h"
#include "XYZ/Renderer/Renderer.h"
#include "XYZ/Renderer/Renderer2D.h"
#include "XYZ/Renderer/SceneRenderer.h"
#include "XYZ/ECS/ComponentGroup.h"
#include "SceneEntity.h"


#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "XYZ/Script/ScriptEngine.h"

namespace XYZ {

	static std::vector<TransformComponent> s_EditTransforms;

	Scene::Scene(const std::string& name)
		:
		m_Name(name),
		m_SelectedEntity(NULL_ENTITY),
		m_CameraEntity(NULL_ENTITY),
		m_PhysicsWorld(glm::vec2(0.0f, -9.8f))
	{
		m_ViewportWidth = 0;
		m_ViewportHeight = 0;

		m_CameraTexture = Texture2D::Create({ TextureWrap::Clamp, TextureParam::Linear, TextureParam::Nearest }, "Assets/Textures/Gui/Camera.png");
		m_CameraSubTexture = Ref<SubTexture>::Create(m_CameraTexture, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		m_CameraMaterial = Ref<Material>::Create(Shader::Create("Assets/Shaders/DefaultShader.glsl"));
		m_CameraMaterial->Set("u_Color", glm::vec4(1.0f));
		m_CameraMaterial->Set("u_Texture", m_CameraTexture);
		m_CameraRenderer = SpriteRenderer(m_CameraMaterial, m_CameraSubTexture, glm::vec4(1.0f), 0);

		m_RenderView = &m_ECS.CreateView<TransformComponent, SpriteRenderer>();
		m_ParticleView = &m_ECS.CreateView<TransformComponent, ParticleComponent>();
		m_LightView = &m_ECS.CreateView<TransformComponent, PointLight2D>();
		m_AnimatorView = &m_ECS.CreateView<AnimatorComponent>();
		m_RigidBodyView = &m_ECS.CreateView<TransformComponent, RigidBody2DComponent>();

		m_ECS.ForceStorage<ScriptComponent>();
		m_ScriptStorage = m_ECS.GetStorage<ScriptComponent>();
		m_AnimatorStorage = m_ECS.GetStorage<AnimatorComponent>();




		////////////////////////
		Skeleton skeleton;
		skeleton.Bones.push_back({});
		skeleton.Bones.push_back({}); // Left hand
		skeleton.Bones.push_back({}); // Right hand
		skeleton.Bones.push_back({}); // Left knee
		skeleton.Bones.push_back({}); // Right knee
		skeleton.Bones.push_back({}); // Left foot
		skeleton.Bones.push_back({}); // Right foot

		skeleton.Bones[0].Transform = glm::translate(glm::vec3( 0.0f,  0.0f, 0.0f));
		skeleton.Bones[1].Transform = glm::translate(glm::vec3(-1.0f,  0.0f, 0.0f));
		skeleton.Bones[2].Transform = glm::translate(glm::vec3( 1.0f,  0.0f, 0.0f));
		skeleton.Bones[3].Transform = glm::translate(glm::vec3(-0.5f, -1.0f, 0.0f));
		skeleton.Bones[4].Transform = glm::translate(glm::vec3( 0.5f, -1.0f, 0.0f));
		skeleton.Bones[5].Transform = glm::translate(glm::vec3( 0.0f, -1.0f, 0.0f));
		skeleton.Bones[6].Transform = glm::translate(glm::vec3( 0.0f, -1.0f, 0.0f));

		int32_t root		= skeleton.BoneHierarchy.Insert(&skeleton.Bones[0], 0);
		int32_t leftHand	= skeleton.BoneHierarchy.Insert(&skeleton.Bones[1], root);
		int32_t rightHand	= skeleton.BoneHierarchy.Insert(&skeleton.Bones[2], root);
		int32_t leftKnee	= skeleton.BoneHierarchy.Insert(&skeleton.Bones[3], root);
		int32_t rightKnee	= skeleton.BoneHierarchy.Insert(&skeleton.Bones[4], root);
		int32_t leftFoot	= skeleton.BoneHierarchy.Insert(&skeleton.Bones[5], leftKnee);
		int32_t rightFoot	= skeleton.BoneHierarchy.Insert(&skeleton.Bones[6], rightKnee);


		skeleton.Bones[0].ID = root;
		skeleton.Bones[1].ID = leftHand;
		skeleton.Bones[2].ID = rightHand;
		skeleton.Bones[3].ID = leftKnee;
		skeleton.Bones[4].ID = rightKnee;
		skeleton.Bones[5].ID = leftFoot;
		skeleton.Bones[6].ID = rightFoot;

		skeleton.Bones[0].Name = "Root";
		skeleton.Bones[1].Name = "Left Hand";
		skeleton.Bones[2].Name = "Right Hand";
		skeleton.Bones[3].Name = "Left Knee";
		skeleton.Bones[4].Name = "Right Knee";
		skeleton.Bones[5].Name = "Left Foot";
		skeleton.Bones[6].Name = "Right Foot";


		glm::vec2 texCoords[4] = {
		    {0.0f, 0.0f},
		    {1.0f, 0.0f},
		    {1.0f, 1.0f},
		    {0.0f, 1.0f}
		};
		glm::vec3 firstQuad[4] = {
			{-0.5f, -0.5f, 0.0f},
			{ 0.5f, -0.5f, 0.0f},
			{ 0.5f,  0.5f, 0.0f},
			{-0.5f,  0.5f, 0.0f},
		};

		m_SkeletalMesh = new SkeletalMesh(skeleton, Ref<Material>::Create(Shader::Create("Assets/Shaders/SkeletalAnimationShader.glsl")));
		
		VertexBoneData data;
		data.Weights[0] = 1.0f;
		data.IDs[0] = 5;
		m_SkeletalMesh->m_Vertices.push_back({ firstQuad[0], texCoords[0], data });
		m_SkeletalMesh->m_Vertices.push_back({ firstQuad[1], texCoords[1], data });
		data.IDs[0] = 3;
		m_SkeletalMesh->m_Vertices.push_back({ firstQuad[2], texCoords[2], data });
		m_SkeletalMesh->m_Vertices.push_back({ firstQuad[3], texCoords[3], data });

		m_SkeletalMesh->m_Indices.push_back(0);
		m_SkeletalMesh->m_Indices.push_back(1);
		m_SkeletalMesh->m_Indices.push_back(2);
		m_SkeletalMesh->m_Indices.push_back(2);
		m_SkeletalMesh->m_Indices.push_back(3);
		m_SkeletalMesh->m_Indices.push_back(0);
		m_SkeletalMesh->RebuildBuffers();

		{
			//KeyFrame::Data data;
			//data.Joint = &anim.Skeleton.Joints[0];
			//data.StartPosition = glm::vec3(0.0f);
			//data.EndPosition = glm::vec3(1.0f);
			//anim.KeyFrames.push_back({});
			//anim.KeyFrames.back().AffectedJoints.push_back(data);
			//anim.KeyFrames.back().Length = 5.0f;
		}
		{
			//KeyFrame::Data data;
			//data.Joint = &anim.Skeleton.Joints[1];
			//data.StartPosition = glm::vec3(0.0f,-4.0f, 0.0f);
			//data.EndPosition = glm::vec3(1.0f);
			//anim.KeyFrames.push_back({});
			//anim.KeyFrames.back().AffectedJoints.push_back(data);
			//anim.KeyFrames.back().Length = 5.0f;
			//
			//data.Joint = &anim.Skeleton.Joints[2];
			//data.StartPosition = glm::vec3(0.0f, -4.0f, 0.0f);
			//data.EndPosition = glm::vec3(-1.0f);
			//anim.KeyFrames.back().AffectedJoints.push_back(data);
		}
		
	}

	Scene::~Scene()
	{
		delete m_SkeletalMesh;
	}

	SceneEntity Scene::CreateEntity(const std::string& name, const GUID& guid)
	{
		SceneEntity entity(m_ECS.CreateEntity(), this);
		IDComponent id;
		id.ID = guid;
		entity.AddComponent<IDComponent>(id);
		entity.AddComponent<Relationship>(Relationship());
		entity.AddComponent<SceneTagComponent>(SceneTagComponent(name));
		entity.AddComponent<TransformComponent>(TransformComponent(glm::vec3(0.0f, 0.0f, 0.0f)));


		m_Entities.push_back(entity);
		return entity;
	}

	void Scene::DestroyEntity(SceneEntity entity)
	{
		uint32_t lastEntity = m_Entities.back();
		if (entity.m_ID == m_SelectedEntity)
			m_SelectedEntity = NULL_ENTITY;
		// Swap with last and delete
		auto it = std::find(m_Entities.begin(), m_Entities.end(), (uint32_t)entity);
		if (it != m_Entities.end())
		{
			*it = std::move(m_Entities.back());
			m_Entities.pop_back();
		}
		m_ECS.DestroyEntity(entity);
	}

	void Scene::OnPlay()
	{
		bool foundCamera = false;
		s_EditTransforms.clear();
		s_EditTransforms.resize(m_ECS.GetNumberOfEntities());		
		for (auto entity : m_Entities)
		{
			Entity ent(entity, &m_ECS);
			s_EditTransforms[entity] = ent.GetComponent<TransformComponent>();
			if (ent.HasComponent<CameraComponent>())
			{
				ent.GetStorageComponent<CameraComponent>().Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
				m_CameraEntity = ent;
				foundCamera = true;
			}
			if (m_ECS.Contains<RigidBody2DComponent>(entity))
			{
				RigidBody2DComponent& rigidBody = m_ECS.GetComponent<RigidBody2DComponent>(entity);
				TransformComponent& transform = m_ECS.GetComponent<TransformComponent>(entity);
				rigidBody.Body = m_PhysicsWorld.CreateBody(glm::vec2(transform.Translation.x, transform.Translation.y), 0.0f);

				if (rigidBody.Type == RigidBody2DComponent::BodyType::Static)
					rigidBody.Body->SetType(PhysicsBody::Type::Static);
				else if (rigidBody.Type == RigidBody2DComponent::BodyType::Dynamic)
					rigidBody.Body->SetType(PhysicsBody::Type::Dynamic);
				else if (rigidBody.Type == RigidBody2DComponent::BodyType::Kinematic)
					rigidBody.Body->SetType(PhysicsBody::Type::Kinematic);

				if (m_ECS.Contains<BoxCollider2DComponent>(entity))
				{
					BoxCollider2DComponent& boxCollider = m_ECS.GetComponent<BoxCollider2DComponent>(entity);
					boxCollider.Shape = m_PhysicsWorld.AddBox2DShape(
						rigidBody.Body,
						-boxCollider.Size / 2.0f,
						boxCollider.Size / 2.0f,
						boxCollider.Density
					);
				}
			}
		}
		if (!foundCamera)
		{
			XYZ_LOG_ERR("No camera found in the scene");
			return;
		}
		for (size_t i = 0; i < m_ScriptStorage->Size(); ++i)
		{
			ScriptComponent& script = (*m_ScriptStorage)[i];
			ScriptEngine::OnCreateEntity({ m_ScriptStorage->GetEntityAtIndex(i), this });
		}
	}

	void Scene::OnStop()
	{
		for (auto entity : m_Entities)
		{
			Entity ent(entity, &m_ECS);
			ent.GetComponent<TransformComponent>() = s_EditTransforms[entity];
			if (m_ECS.Contains<RigidBody2DComponent>(entity))
			{
				RigidBody2DComponent& rigidBody = m_ECS.GetComponent<RigidBody2DComponent>(entity);
				m_PhysicsWorld.DestroyBody(rigidBody.Body);
				rigidBody.Body = nullptr;
				if (m_ECS.Contains<BoxCollider2DComponent>(entity))
				{
					BoxCollider2DComponent& boxCollider = m_ECS.GetComponent<BoxCollider2DComponent>(entity);
					m_PhysicsWorld.DestroyShape(boxCollider.Shape);
					boxCollider.Shape = nullptr;
				}
			}
		}
	}

	void Scene::OnRender()
	{
		// 3D part here

		///////////////
		Entity cameraEntity(m_CameraEntity, &m_ECS);
		SceneRendererCamera renderCamera;
		auto& cameraComponent = cameraEntity.GetStorageComponent<CameraComponent>();
		auto& cameraTransform = cameraEntity.GetStorageComponent<TransformComponent>();
		renderCamera.Camera = cameraComponent.Camera;
		renderCamera.ViewMatrix = glm::inverse(cameraTransform.GetTransform());

		SceneRenderer::GetOptions().ShowGrid = false;
		SceneRenderer::BeginScene(this, renderCamera);
		for (size_t i = 0; i < m_RenderView->Size(); ++i)
		{
			auto [transform, renderer] = (*m_RenderView)[i];
			SceneRenderer::SubmitSprite(&renderer, &transform);
		}
		for (size_t i = 0; i < m_ParticleView->Size(); ++i)
		{
			auto [transform, particle] = (*m_ParticleView)[i];
			SceneRenderer::SubmitParticles(&particle, &transform);
		}

		for (size_t i = 0; i < m_LightView->Size(); ++i)
		{
			auto [transform, light] = (*m_LightView)[i];
			SceneRenderer::SubmitLight(&light, transform.GetTransform());
		}
		
		SceneRenderer::EndScene();
	}

	void Scene::OnUpdate(Timestep ts)
	{
		m_PhysicsWorld.Update(ts);
		
		for (size_t i = 0; i < m_ScriptStorage->Size(); ++i)
		{
			ScriptComponent& scriptComponent = (*m_ScriptStorage)[i];
			ScriptEngine::OnUpdateEntity({ m_ScriptStorage->GetEntityAtIndex(i),this }, ts);
		}

		for (size_t i = 0; i < m_AnimatorStorage->Size(); ++i)
		{
			AnimatorComponent& anim = (*m_AnimatorStorage)[i];
			anim.Controller.Update(ts);
		}

		for (size_t i = 0; i < m_AnimatorView->Size(); ++i)
		{
			auto [animator] = (*m_AnimatorView)[i];
			animator.Controller.Update(ts);
		}

		for (size_t i = 0; i < m_ParticleView->Size(); ++i)
		{
			auto [transform, particle] = (*m_ParticleView)[i];
			auto material = particle.ComputeMaterial->GetParentMaterial();
			auto materialInstance = particle.ComputeMaterial;

			materialInstance->Set("u_Time", ts);
			materialInstance->Set("u_ParticlesInExistence", (int)std::ceil(particle.ParticleEffect->GetEmittedParticles()));

			material->GetShader()->Bind();
			materialInstance->Bind();

			particle.ParticleEffect->Update(ts);
			material->GetShader()->Compute(32, 32, 1);
		}

		
 		for (size_t i = 0; i < m_RigidBodyView->Size(); ++i)
		{
			auto [transform, rigidBody] = (*m_RigidBodyView)[i];
			transform.Translation.x = rigidBody.Body->GetPosition().x;
			transform.Translation.y = rigidBody.Body->GetPosition().y;
			//transform.Rotation.z = rigidBody.Body->GetAngle();
		}
	}

	void Scene::OnRenderEditor(const EditorCamera& camera)
	{
		SceneRenderer::BeginScene(this, camera.GetViewProjection());
		
		if (m_SelectedEntity != NULL_ENTITY)
		{
			if (m_ECS.Contains<CameraComponent>(m_SelectedEntity))
			{
				SceneRenderer::SubmitSprite(&m_CameraRenderer, &m_ECS.GetComponent<TransformComponent>(m_SelectedEntity));
				showCamera(m_SelectedEntity);
			}
			else
				showSelection(m_SelectedEntity);
		}
		for (size_t i = 0; i < m_RenderView->Size(); ++i)
		{
			auto [transform, renderer] = (*m_RenderView)[i];
			if (renderer.IsVisible && renderer.SubTexture.Raw())
				SceneRenderer::SubmitSprite(&renderer, &transform);
		}
		for (size_t i = 0; i < m_ParticleView->Size(); ++i)
		{
			auto [transform, particle] = (*m_ParticleView)[i];
			SceneRenderer::SubmitParticles(&particle, &transform);
		}
		
		for (size_t i = 0; i < m_LightView->Size(); ++i)
		{
			auto [transform, light] = (*m_LightView)[i];
			SceneRenderer::SubmitLight(&light, transform.GetTransform());
		}
		m_SkeletalMesh->Update(0.01f);
		SceneRenderer::SubmitSkeletalMesh(m_SkeletalMesh);
		
		
		SceneRenderer::EndScene();
	}

	void Scene::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
	}

	SceneEntity Scene::GetEntity(uint32_t index)
	{
		return { m_Entities[index], this };
	}

	SceneEntity Scene::GetSelectedEntity()
	{
		return { m_SelectedEntity, this };
	}

	void Scene::showSelection(uint32_t entity)
	{
		auto transformComponent = m_ECS.GetComponent<TransformComponent>(entity);
		auto& translation = transformComponent.Translation;
		auto& scale = transformComponent.Scale;

		glm::vec3 topLeft = { translation.x - scale.x / 2,translation.y + scale.y / 2, translation.z };
		glm::vec3 topRight = { translation.x + scale.x / 2,translation.y + scale.y / 2, translation.z };
		glm::vec3 bottomLeft = { translation.x - scale.x / 2,translation.y - scale.y / 2, translation.z };
		glm::vec3 bottomRight = { translation.x + scale.x / 2,translation.y - scale.y / 2, translation.z };
	
		Renderer2D::SubmitLine(topLeft, topRight);
		Renderer2D::SubmitLine(topRight, bottomRight);
		Renderer2D::SubmitLine(bottomRight, bottomLeft);
		Renderer2D::SubmitLine(bottomLeft, topLeft);
	}

	void Scene::showCamera(uint32_t entity)
	{
		auto& camera = m_ECS.GetComponent<CameraComponent>(entity).Camera;
		auto transformComponent = m_ECS.GetComponent<TransformComponent>(entity);

		auto& translation = transformComponent.Translation;
		if (camera.GetProjectionType() == CameraProjectionType::Orthographic)
		{
			float size = camera.GetOrthographicProperties().OrthographicSize;
			float aspect = (float)m_ViewportWidth / (float)m_ViewportHeight;
			float width = size * aspect;
			float height = size;

			glm::vec3 topLeft = { translation.x - width / 2.0f,translation.y + height / 2.0f,1.0f };
			glm::vec3 topRight = { translation.x + width / 2.0f,translation.y + height / 2.0f,1.0f };
			glm::vec3 bottomLeft = { translation.x - width / 2.0f,translation.y - height / 2.0f,1.0f };
			glm::vec3 bottomRight = { translation.x + width / 2.0f,translation.y - height / 2.0f,1.0f };

			Renderer2D::SubmitLine(topLeft, topRight);
			Renderer2D::SubmitLine(topRight, bottomRight);
			Renderer2D::SubmitLine(bottomRight, bottomLeft);
			Renderer2D::SubmitLine(bottomLeft, topLeft);
		}
	}

}