#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "ParticleEffect2D.h"

#include "XYZ/Renderer/Shader.h"
#include "XYZ/Renderer/Material.h"


namespace XYZ {
	/*! @class ParticleSystem2D 
	*	@brief Implements a 2D particle system
	*/
	class ParticleSystem2D : public System
	{
	public:
		ParticleSystem2D();
		void Update(float dt);
		virtual void Add(uint32_t entity) override;
		virtual void Remove(uint32_t entity) override;
		virtual bool Contains(uint32_t entity) override;

	private:
		struct Component : public System::Component
		{
			ParticleEffect2D* Effect;
		};

		std::vector<Component> m_Components;
		bool m_Sorted = true;
	};


}