#pragma once
#include "XYZ/ECS/ECSManager.h"


namespace XYZ {
	class AudioSystem : public System
	{
	public:
		AudioSystem();
		void Update(float dt);
		virtual void Add(uint32_t entity) override;
		virtual void Remove(uint32_t entity) override;
		virtual bool Contains(uint32_t entity) override;

	private:
		struct Component : public System::Component
		{
			int audioIndex;
		};

		std::vector<Component> m_Components;
	};
}