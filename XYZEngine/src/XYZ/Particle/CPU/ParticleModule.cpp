#include "stdafx.h"
#include "ParticleModule.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Renderer/SceneRenderer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
namespace XYZ {

	static float CalcRatio(float length, float value)
	{
		return (length - value) / length;
	}

	MainModule::MainModule()
		:
		m_Enabled(true)
	{
	}

	void MainModule::UpdateParticles(float ts, ParticleDataBuffer& data) const
	{
		if (m_Enabled)
		{
			uint32_t aliveParticles = data.GetAliveParticles();
			for (uint32_t i = 0; i < aliveParticles; ++i)
			{
				data.m_Particle[i].Position += data.m_Particle[i].Velocity * ts;
				data.m_Particle[i].LifeRemaining -= ts;
				if (data.m_Particle[i].LifeRemaining <= 0.0f)
				{
					data.Kill(i);
					aliveParticles--;
				}
			}
		}
	}

	void MainModule::SetEnabled(bool enabled)
	{
		m_Enabled = enabled;
	}

	LightModule::LightModule()
		:
		m_MaxLights(1000),
		m_Enabled(true)
	{
		
	}
	void LightModule::UpdateParticles(float ts, ParticleDataBuffer& data)
	{
		if (m_Enabled)
		{
			m_Lights.clear();
			uint32_t aliveParticles = data.GetAliveParticles();
			if (m_TransformEntity.IsValid()
				&& m_LightEntity.IsValid()
				&& m_LightEntity.HasComponent<PointLight2D>())
			{
				for (uint32_t i = 0; i < aliveParticles && i < m_MaxLights; ++i)
				{
					m_Lights.push_back(data.m_Particle[i].Position);
				}
			}
		}
	}

	void LightModule::Reset()
	{
		if (m_Enabled)
		{
			SetEnabled(false);
			SetEnabled(true);
		}
	}

	void LightModule::SetEnabled(bool enabled)
	{
		m_Enabled = enabled;
		if (!m_Enabled)
			m_Lights = std::vector<glm::vec3>(); // Release memory
	}

	TextureAnimationModule::TextureAnimationModule()
		:
		m_Tiles(1, 1),
		m_StartFrame(0),
		m_CycleLength(5.0f),
		m_Enabled(true)
	{
	}
	void TextureAnimationModule::UpdateParticles(float ts, ParticleDataBuffer& data) const
	{
		if (m_Enabled)
		{
			uint32_t stageCount = m_Tiles.x * m_Tiles.y;
			float columnSize	= 1.0f / m_Tiles.x;
			float rowSize		= 1.0f / m_Tiles.y;
			
			uint32_t aliveParticles = data.GetAliveParticles();
			for (uint32_t i = 0; i < aliveParticles; ++i)
			{
				float ratio			= CalcRatio(m_CycleLength, data.m_Particle[i].LifeRemaining);
				float stageProgress = ratio * stageCount;
				
				uint32_t index  = (uint32_t)floor(stageProgress);
				float column	= index % m_Tiles.x;
				float row		= index / m_Tiles.y;
				
				data.m_TexOffset[i] = glm::vec2(column / m_Tiles.x, row / m_Tiles.y);
			}
		}
	}
	void TextureAnimationModule::SetEnabled(bool enabled)
	{
		m_Enabled = enabled;
	}
	RotationOverLife::RotationOverLife()
		:
		m_EulerAngles(0.0f),
		m_CycleLength(5.0f),
		m_Enabled(true)
	{
	}
	void RotationOverLife::UpdateParticles(float ts, ParticleDataBuffer& data) const
	{
		if (m_Enabled)
		{
			uint32_t aliveParticles = data.GetAliveParticles();
			glm::vec3 radians = glm::radians(m_EulerAngles);
			for (uint32_t i = 0; i < aliveParticles; ++i)
			{
				float ratio = CalcRatio(m_CycleLength, data.m_Particle[i].LifeRemaining);
				data.m_Rotation[i] = glm::quat(radians * ratio);
			}
		}
	}
	void RotationOverLife::SetEnabled(bool enabled)
	{
		m_Enabled = enabled;
	}
	PhysicsModule::PhysicsModule()
		:
		m_Shape(PhysicsModule::Shape::Circle),
		m_BoxSize(0.5f),
		m_Radius(0.5f),
		m_Density(1.0f),
		m_Friction(0.0f),
		m_Restitution(0.0f),
		m_PhysicsWorld(nullptr),
		m_MaxParticles(0),
		m_Enabled(true)
	{
	}
	void PhysicsModule::Generate(ParticleDataBuffer& data, uint32_t startId, uint32_t endId, const glm::mat4& transform)
	{
		if (m_Bodies.size() >= endId)
		{
			if (endId > startId)
			{	
				ScopedLock<b2World> world = m_PhysicsWorld->GetWorld(); // Lock is required
				for (uint32_t i = startId; i < endId; ++i)
				{
					glm::vec4 translation = transform * glm::vec4(data.m_Particle[i].Position, 1.0f);
					glm::mat4 particleTransform = transform * glm::translate(data.m_Particle[i].Position);
					auto [trans, rot, scale] = TransformComponent::DecomposeTransformToComponents(particleTransform);
					b2Vec2 position = {
						   translation.x,
						   translation.y
					};
					b2Vec2 vel = {
						   data.m_Particle[i].Velocity.x,
						   data.m_Particle[i].Velocity.y,
					};
					m_Bodies[i]->SetEnabled(true);
					m_Bodies[i]->SetTransform(position, 0.0f);
					m_Bodies[i]->SetLinearVelocity(vel);
				}
			}
		}
	}
	void PhysicsModule::UpdateParticles(ParticleDataBuffer& data, const glm::mat4& transform) const
	{
		if (m_Enabled)
		{
			uint32_t aliveParticles = data.GetAliveParticles();
			if (m_Bodies.size() >= aliveParticles)
			{
				auto [trans, rot, scale] = TransformComponent::DecomposeTransformToComponents(transform);
				for (uint32_t i = 0; i < aliveParticles; ++i)
				{		
					auto point = m_Bodies[i]->GetLocalPoint({ trans.x, trans.y });
					data.m_Particle[i].Position.x = point.x;
					data.m_Particle[i].Position.y = point.y;
				}
			}
		}
	}
	void PhysicsModule::SetPhysicsWorld(PhysicsWorld2D* world)
	{
		if (m_PhysicsWorld != world)
		{
			m_PhysicsWorld = world;
			if (m_PhysicsWorld)
			{
				ScopedLock<b2World> physicsWorld = m_PhysicsWorld->GetWorld();
				generateBodies(&physicsWorld.As());
			}
		}
	}
	void PhysicsModule::SetMaxParticles(uint32_t count)
	{
		m_MaxParticles = count;
		if (m_Enabled && m_PhysicsWorld)
		{
			ScopedLock<b2World> physicsWorld = m_PhysicsWorld->GetWorld();
			generateBodies(&physicsWorld.As());
		}
	}
	void PhysicsModule::Reset()
	{
		if (m_Enabled)
		{
			SetEnabled(false); // Destroys bodies
			SetEnabled(true);  // Generate new bodies
		}
	}
	void PhysicsModule::SetEnabled(bool enabled)
	{
		if (m_Enabled == enabled)
			return;
		
		if (m_PhysicsWorld)
		{
			ScopedLock<b2World> physicsWorld = m_PhysicsWorld->GetWorld();
			if (!enabled)
				destroyBodies(&physicsWorld.As());
			else
				generateBodies(&physicsWorld.As());
		}
		m_Enabled = enabled;
	}

	void PhysicsModule::generateBodies(b2World* world)
	{
		destroyBodies(world);
		if (m_Bodies.size() != m_MaxParticles)
			m_Bodies.resize(m_MaxParticles);
		
		const PhysicsWorld2D::Layer particleLayer = m_PhysicsWorld->GetLayer(PhysicsWorld2D::ParticleLayer);

		b2FixtureDef fd;
		fd.filter.categoryBits = particleLayer.m_CollisionMask.to_ulong();
		fd.filter.maskBits	   = BIT(particleLayer.m_ID);
		fd.density			   = m_Density;
		fd.friction			   = m_Friction;
		fd.restitution		   = m_Restitution;


		b2BodyDef bd;
		bd.type = b2_dynamicBody;
		bd.enabled = false;
		
		const b2Shape* shape = prepareShape();
		for (auto& body : m_Bodies)
		{		
			body = world->CreateBody(&bd);	
			fd.shape = shape;
			body->CreateFixture(&fd);
			body->SetGravityScale(0.0f);
		}
	}
	void PhysicsModule::destroyBodies(b2World* world)
	{
		for (auto body : m_Bodies)
			world->DestroyBody(body);
		m_Bodies.clear();
	}
	const b2Shape* PhysicsModule::prepareShape()
	{
		b2Shape* shape = nullptr;
		switch (m_Shape)
		{
		case XYZ::PhysicsModule::Shape::Circle:
			m_CircleShape.m_radius = m_Radius;
			shape = &m_CircleShape;
			break;
		case XYZ::PhysicsModule::Shape::Box:
			m_BoxShape.SetAsBox(m_BoxSize.x, m_BoxSize.y);
			shape = &m_BoxShape;
			break;
		default:
			break;
		}
		return shape;
	}
}