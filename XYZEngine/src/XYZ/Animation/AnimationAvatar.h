#pragma once

#include "XYZ/Scene/SceneEntity.h"
#include "Property.h"


namespace XYZ {
	class Animation;
	class AnimationAvatar
	{
	public:
		void SetAvatar();
		bool Create(const SceneEntity& entity, const Ref<Animation>& anim);
		
		bool Valid() const;
		bool Compatible(const SceneEntity& entity) const;
	private:

		template <typename T>
		bool propertyCompatible(const std::vector<Property<T>>& props, const SceneEntity& entity) const;

	private:
		using EntityPropertyTree = std::vector<std::pair<SceneEntity, IProperty*>>;
		
		Ref<Animation>     m_Animation;
		SceneEntity        m_Entity;

		EntityPropertyTree m_Properties;

		friend Animation;
	};

	template<typename T>
	inline bool AnimationAvatar::propertyCompatible(const std::vector<Property<T>>& props, const SceneEntity& entity) const
	{
		for (const auto& pr : props)
		{
			if (pr.IsCompatible(entity))
				return true;
		}
		return false;
	}

}