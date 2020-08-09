#pragma once
#include "XYZ/ECS/ECSManager.h"
#include "SpriteAnimation.h"
#include "SpriteRenderer.h"

namespace XYZ {

	/**
	* @class SpriteSystem
	* @brief Updates entities with components Renderable2D and SpriteAnimation
	*/
	class SpriteSystem : public System
	{
	public:
		/**
		* Construct a sprite system, set signature
		*/
		SpriteSystem(ECSManager*ecs);

		/**
		* Update stored animations
		* @param dt		delta time
		*/
		virtual void Update(float dt);

		/**
		* Add entity to the vector of components
		* @param[in] entity
		*/
		virtual void Add(uint32_t entity) override;
		/**
		* Remove entity from the vector of components
		*/
		virtual void Remove(uint32_t entity) override;
		/**
		* @return true if the vector of components contains entity
		*/
		virtual bool Contains(uint32_t entity) override;

	private:
		struct Component : public System::Component
		{
			SpriteAnimation* Animation;
			SpriteRenderer* Renderable;
		};
		ECSManager* m_ECS;

		std::vector<Component> m_Components;
	};
}