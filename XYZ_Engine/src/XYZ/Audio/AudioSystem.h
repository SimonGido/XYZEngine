#pragma once
#include "XYZ/ECS/ECSManager.h"


namespace XYZ {
	class AudioSystem : public System
	{
	public:
		AudioSystem();
		void Update(float dt);
		virtual void Add(Entity entity) override;
		virtual void Remove(Entity entity) override;
		virtual bool Contains(Entity entity) override;

	private:
		struct Component : public System::Component
		{
			int audioIndex;
		};

		std::vector<Component> m_Components;
	};
}