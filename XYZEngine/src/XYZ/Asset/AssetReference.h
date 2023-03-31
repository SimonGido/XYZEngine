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

		AssetReference(const AssetHandle& handle)
			:
			m_Handle(handle)
		{
		}
		AssetReference(const Ref<T>& asset)
			: m_Asset(asset)
		{
			static_assert(std::is_base_of<Asset, T>::value, "Asset reference can reference only assets");
			if (m_Asset.Raw())
				m_Handle = asset->GetHandle();
		}

		AssetReference& operator=(const Ref<T>& asset)
		{
			m_Asset = asset;
			return *this;
		}

		const AssetHandle& GetHandle() const { return m_Handle; }

		Ref<T>& operator->()				{ return ensure(); }
		const Ref<T>& operator->() const	{ return ensure(); }

		Ref<T>&		  Value()				{ return ensure(); }
		const Ref<T>& Value() const			{ return ensure(); }
		bool		  Valid() const			{ return AssetManager::Exist(m_Handle); }

	private:
		Ref<T>& ensure();
		const Ref<T>& ensure() const;

	private:
		AssetHandle	   m_Handle;
		mutable Ref<T> m_Asset;
	};


	template<typename T>
	inline Ref<T>& AssetReference<T>::ensure()
	{
		static_assert(std::is_base_of<Asset, T>::value, "Asset reference can reference only assets");
		// Asset reference is not stored or it has been reloaded
		if (!m_Asset.Raw() || m_Asset->IsFlagSet(AssetFlag::Reloaded))
		{
			// We have valid asset handle
			if (AssetManager::Exist(m_Handle))
				m_Asset = AssetManager::GetAsset<T>(m_Handle);
		}
		return m_Asset;
	}

	template<typename T>
	inline const Ref<T>& AssetReference<T>::ensure() const
	{
		static_assert(std::is_base_of<Asset, T>::value, "Asset reference can reference only assets");
		// Asset reference is not stored or it has been reloaded
		if (!m_Asset.Raw() || m_Asset->IsFlagSet(AssetFlag::Reloaded))
		{
			// We have valid asset handle
			if (AssetManager::Exist(m_Handle))
				m_Asset = AssetManager::GetAsset<T>(m_Handle);
		}
		return m_Asset;
	}
}