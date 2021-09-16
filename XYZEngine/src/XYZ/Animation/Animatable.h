#pragma once
#include "XYZ/Scene/SceneEntity.h"

#include <any>


namespace XYZ {

	class Animatable
	{
		using SetRefCallbackFn = std::function<void(std::any& reference, SceneEntity entity)>;
	public:
		Animatable(SceneEntity entity, const SetRefCallbackFn& setReferenceCallback);

		void SetReference();

		template <typename T>
		T* GetReference();

		template <typename T>
		bool IsType() const;

		const SceneEntity& GetEntity() const { return m_SceneEntity; }
	private:
		std::any		 m_ComponentReference;
		SetRefCallbackFn m_SetRefCallback;
		SceneEntity		 m_SceneEntity;
	};



	template<typename T>
	inline T* Animatable::GetReference()
	{
		XYZ_ASSERT(IsType<T>(), "Invalid type");
		return std::any_cast<T*>(m_ComponentReference);
	}

	template<typename T>
	inline bool Animatable::IsType() const
	{
		try
		{
			std::any_cast<T*>(m_ComponentReference);
		}
		catch (const std::bad_any_cast& e)
		{
			return false;
		}
		return true;
	}

}