#pragma once
#include "XYZ/Scene/SceneEntity.h"
#include "XYZ/ECS/ComponentWrapper.h"

namespace XYZ {


	class Animatable
	{
	public:
		Animatable(SceneEntity entity);

		void SetReference();

		template <typename T>
		T&   GetReference();

		template <typename T>
		bool IsType() const;

		const SceneEntity& GetEntity() const { return m_SceneEntity; }
	private:
		AnyComponentReference m_ComponentReference;
		SceneEntity			  m_SceneEntity;
	};

	template<typename T>
	inline T& Animatable::GetReference()
	{
		XYZ_ASSERT(IsType<T>(), "Invalid type");
		return m_ComponentReference.Get<T>();
	}

	template<typename T>
	inline bool Animatable::IsType() const
	{
		return m_ComponentReference.IsStored<T>();
	}

}