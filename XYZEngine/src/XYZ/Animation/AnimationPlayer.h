#pragma once

#include "XYZ/Scene/SceneEntity.h"

namespace XYZ {
	class Animation;
	class IProperty;

	class AnimationPlayer
	{
	public:
		AnimationPlayer();

		void Update(Timestep ts);
		bool Create(const SceneEntity& entity, const Ref<Animation>& anim);
		
		void Reset();
		void SetCurrentFrame(uint32_t frame);

		bool Valid() const;
		bool Compatible(SceneEntity entity) const;

		uint32_t GetCurrentFrame() const;

	private:
		struct PropertyNode
		{
			SceneEntity Entity;
			IProperty*  Property;
			size_t		Key;
		};
		Ref<Animation>     m_Animation;
		SceneEntity        m_Entity;
		float			   m_CurrentTime;

		std::vector<PropertyNode> m_Properties;
		bool					  m_IsCreated = false;
		friend Animation;
	};

}