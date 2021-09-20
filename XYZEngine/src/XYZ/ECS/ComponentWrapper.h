#pragma once
#include "ECSManager.h"


namespace XYZ {
	class AnyComponentReference
	{
	public:
		AnyComponentReference();

		template <typename T>
		AnyComponentReference& operator=(T& component);

		template <typename T>
		T& Get();

		template <typename T>
		const T& Get() const;

		template <typename T>
		bool IsStored() const;

	private:
		void*    m_Reference;
		uint16_t m_ID;
	};

	template<typename T>
	inline AnyComponentReference& AnyComponentReference::operator=(T& component)
	{
		XYZ_ASSERT(Component<T>::Registered(), "Component is not registered");
		m_Reference = &component;
		m_ID = Component<T>::ID();
		return *this;
	}
	template<typename T>
	inline T& AnyComponentReference::Get()
	{
		XYZ_ASSERT(Component<T>::ID() == m_ID, "Accessed type is not stored");
		return static_cast<T>(*m_Reference);
	}
	template<typename T>
	inline const T& AnyComponentReference::Get() const
	{
		XYZ_ASSERT(Component<T>::ID() == m_ID, "Accessed type is not stored");
		return static_cast<T>(*m_Reference);
	}
	template<typename T>
	inline bool AnyComponentReference::IsStored() const
	{
		XYZ_ASSERT(Component<T>::Registered(), "Component is not registered");
		return (Component<T>::ID() == m_ID);
	}

}