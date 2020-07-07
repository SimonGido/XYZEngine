#pragma once

#include <XYZ.h>


class GameLayer : public XYZ::Layer
{
public:
	GameLayer();
	virtual ~GameLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(float ts) override;
	virtual void OnEvent(XYZ::Event& event) override;

private:

	void InitBackgroundParticles(XYZ::Entity entity);
private:
	std::shared_ptr<XYZ::SpriteRenderer> m_RendererSystem;
	std::shared_ptr<XYZ::PhysicsSystem> m_PhysicsSystem;
	std::shared_ptr<XYZ::ParticleSystem2D> m_ParticleSystem;
	std::shared_ptr<XYZ::SpriteSystem> m_SpriteSystem;
	std::shared_ptr<XYZ::InterpolatedMovementSystem> m_InterSystem;
	std::shared_ptr<XYZ::GridCollisionSystem> m_GridCollisionSystem;
	std::shared_ptr<XYZ::RealGridCollisionSystem> m_RealGridCollisionSystem;

	std::shared_ptr<XYZ::Material> m_Material;
	std::shared_ptr<XYZ::Material> m_ParticleMaterial;
	std::shared_ptr<XYZ::OrthoCameraController> m_CameraController;

	XYZ::Entity m_World;
	XYZ::Entity m_Player;
	XYZ::Entity m_PlayerChild;
	XYZ::Entity m_PlayerChild2;

	XYZ::Entity m_ParticleEntity;


	XYZ::Tree<XYZ::Transform2D*> m_TransformTree;
	XYZ::Transform2D* m_WorldTransform;
	XYZ::Transform2D* m_PlayerTransform;
	XYZ::Transform2D* m_PlayerChildTransform;
	XYZ::Transform2D* m_PlayerChildTransform2;

	XYZ::Renderable2D* m_PlayerRenderable;
	XYZ::Renderable2D* m_PlayerChildRenderable;
	XYZ::Renderable2D* m_PlayerChildRenderable2;

	int m_PlayableArea = 20;


	std::shared_ptr<XYZ::AudioSource> m_Audio;

	std::shared_ptr<XYZ::Material> m_FluidMaterial;
	std::shared_ptr<XYZ::FrameBuffer> m_FrameBuffer;


	struct SceneObject
	{
		XYZ::Renderable2D* Renderable;
		XYZ::Transform2D* Transform;
	};


	struct SceneSetup
	{
		void operator()(SceneObject& parent, SceneObject& child)
		{
			child.Transform->SetParent(parent.Transform);
		}
	};

	struct ScenePropagation
	{
		std::shared_ptr<XYZ::SpriteRenderer> RendererSystem;

		void operator()(SceneObject& parent, SceneObject& child)
		{
			child.Transform->CalculateWorldTransformation();
			RendererSystem->PushRenderable(child.Renderable, child.Transform);
		}
	};

	ScenePropagation m_Propagation;
	XYZ::Tree<SceneObject> m_SceneTree;

};