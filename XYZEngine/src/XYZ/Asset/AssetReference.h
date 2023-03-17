#pragma once

#include "Asset.h"
#include "AssetManager.h"


namespace XYZ {
	template <typename T>
	class XYZ_API AssetReference
	{
	public:
		AssetReference()
		{
			static_assert(std::is_base_of<Asset, T>::value, "Asset reference can reference only assets");
		}

		AssetReference(const Ref<T>& asset)
			: m_Asset(asset)
		{
			static_assert(std::is_base_of<Asset, T>::value, "Asset reference can reference only assets");
		}

		AssetReference& operator=(const Ref<T>& asset)
		{
			m_Asset = asset;
			return *this;
		}


		Ref<T>& operator->()				{ return ensure(); }
		const Ref<T>& operator->() const	{ return ensure(); }

		Ref<T>&		  Value()				{ return ensure(); }
		const Ref<T>& Value() const			{ return ensure(); }
		bool		  Valid() const			{ return m_Asset.Raw() != nullptr && m_Asset->IsValid(); }

	private:
		Ref<T>& ensure();
		const Ref<T>& ensure() const;

	private:
		mutable Ref<T> m_Asset;
	};


	template<typename T>
	inline Ref<T>& AssetReference<T>::ensure()
	{
		static_assert(std::is_base_of<Asset, T>::value, "Asset reference can reference only assets");

		static Ref<T> invalidInstance; // Hack to return reference properly
		if (!Valid())
			return invalidInstance;

		if (m_Asset->IsFlagSet(AssetFlag::Reloaded))
			m_Asset = AssetManager::GetAsset<T>(m_Asset->GetHandle());

		return m_Asset;
	}

	template<typename T>
	inline const Ref<T>& AssetReference<T>::ensure() const
	{
		static_assert(std::is_base_of<Asset, T>::value, "Asset reference can reference only assets");
		
		static Ref<T> invalidInstance; // Hack to return reference properly
		if (!Valid())
			return invalidInstance;

		if (m_Asset->IsFlagSet(AssetFlag::Reloaded))
			m_Asset = AssetManager::GetAsset<T>(m_Asset->GetHandle());

		return m_Asset;
	}
}