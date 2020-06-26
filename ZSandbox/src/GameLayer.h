#pragma once

#include <XYZ.h>



struct Propagate
{
	Propagate(XYZ::Transform2D& parent, XYZ::Transform2D& child)
	{
		child.SetParent(&parent);
	}
	Propagate() = default;

	void operator ()(XYZ::Transform2D& parent, XYZ::Transform2D& child)
	{
		if (parent.Updated())
			child.InheritParent(parent);

		std::cout << child.GetPosition().x << std::endl;
	}
};


struct TransformPropagate
{
	// Setup
	TransformPropagate(XYZ::Transform2D* parent, XYZ::Transform2D* child)
	{
		if (parent && child)
			child->SetParent(parent);
	}
	TransformPropagate() = default;
	// Update
	void operator ()(XYZ::Transform2D* parent, XYZ::Transform2D* child)
	{
		child->CalculateWorldTransformation();
		// Should call rendering functions
	}

};


class GameLayer : public XYZ::Layer
{
public:
	GameLayer();
	virtual ~GameLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(float ts) override;
	virtual void OnImGuiRender() override;

private:

	void InitBackgroundParticles(XYZ::Entity entity);
private:
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


	XYZ::Tree<XYZ::Transform2D*, TransformPropagate> m_TransformTree;
	XYZ::Transform2D* m_WorldTransform;
	XYZ::Transform2D* m_PlayerTransform;
	XYZ::Transform2D* m_PlayerChildTransform;
	XYZ::Transform2D* m_PlayerChildTransform2;

	int m_PlayableArea = 20;


	std::shared_ptr<XYZ::AudioSource> m_Audio;

	std::shared_ptr<XYZ::Material> m_FluidMaterial;
	std::shared_ptr<XYZ::FrameBuffer> m_FrameBuffer;
};