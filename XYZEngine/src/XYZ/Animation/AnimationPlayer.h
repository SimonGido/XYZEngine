#pragma once

#include "XYZ/Scene/SceneEntity.h"
#include "Property.h"


namespace XYZ {
	class Animation;
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

		template <typename T>
		bool propertyCompatible(const std::vector<Property<T>>& props, const std::string_view path) const;

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

	template<typename T>
	inline bool AnimationPlayer::propertyCompatible(const std::vector<Property<T>>& props, const std::string_view path) const
	{
		for (const auto& pr : props)
		{
			if (pr.GetPath() == path)
				return true;
		}
		return false;
	}

}