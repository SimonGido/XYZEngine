#pragma once
#include "XYZ/Utils/DataStructures/ThreadPass.h"
#include "XYZ/Core/Timestep.h"

#include <box2d/box2d.h>

#include <glm/glm.hpp>

#include <bitset>

namespace XYZ {

	class PhysicsWorld2D
	{
	public:
		static constexpr uint32_t sc_NumCollisionLayers = 16;
		static constexpr uint32_t sc_NumDefaultLayers = 2;

		using CollisionMask = std::bitset<sc_NumCollisionLayers>;

		enum DefaultLayers { DefaultLayer, ParticleLayer, Num };

		struct Layer
		{
			std::string	  m_Name;
			uint32_t	  m_ID;
			CollisionMask m_CollisionMask;
		};
	public:

		PhysicsWorld2D(const glm::vec2& gravity);

		void Step(Timestep ts);

		void SetLayer(const std::string& name, uint32_t index, const CollisionMask& mask = {});
		
		const Layer& GetLayer(std::string_view name) const;
		const Layer& GetLayer(uint32_t index) const;

		const std::array<Layer, sc_NumCollisionLayers>& GetLayers() const { return m_Layers; }

		ScopedLock<b2World>		GetWorld();
		ScopedLockRead<b2World>	GetWorldRead() const;
	
	private:
		void synchronize();

	private:
		
		b2World					  m_World;
		mutable std::shared_mutex m_Mutex;

		
		std::array<Layer, sc_NumCollisionLayers> m_Layers;
	};
}