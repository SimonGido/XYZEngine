#include "stdafx.h"
#include "ParticleModule.h"

#include "XYZ/Scene/Components.h"
#include "XYZ/Renderer/SceneRenderer.h"

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
		m_PhysicsWorld(nullptr),
		m_MaxParticles(0),
		m_Enabled(true)
	{
	}
	void PhysicsModule::Generate(ParticleDataBuffer& data, uint32_t startId, uint32_t endId)
	{
		if (m_Bodies.size() >= endId)
		{
			for (uint32_t i = startId; i < endId; ++i)
			{
				b2Vec2 position = {
					   data.m_Particle[i].Position.x,
					   data.m_Particle[i].Position.y
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
	void PhysicsModule::UpdateParticles(ParticleDataBuffer& data) const
	{
		if (m_Enabled)
		{
			uint32_t aliveParticles = data.GetAliveParticles();
			if (m_Bodies.size() >= aliveParticles)
			{
				for (uint32_t i = 0; i < aliveParticles; ++i)
				{
					data.m_Particle[i].Position.x = m_Bodies[i]->GetPosition().x;
					data.m_Particle[i].Position.y = m_Bodies[i]->GetPosition().y;
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

		for (auto& body : m_Bodies)
		{
			b2BodyDef bd;
			bd.type = b2_dynamicBody;
			bd.enabled = false;
			body = world->CreateBody(&bd);

			b2PolygonShape shape;
			shape.SetAsBox(1.0f / 2.0f, 1.0f / 2.0f);

			//b2CircleShape shape;
			//shape.m_radius = 0.5f;
			b2FixtureDef fd;
			fd.density = 1;
			fd.shape = &shape;
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
}