#pragma once

#include "Asset.h"


namespace XYZ {
	struct PathHasher
	{
		std::size_t operator()(const std::filesystem::path& path) const
		{
			return hash_value(path);
		}
	};


	struct XYZ_API AssetData : public RefCount
	{
		WeakRef<Asset>	 Asset;
		AssetMetadata	 Metadata;
		

		void WaitUnlock() const;
		
		bool TryLock() const;

		void SpinLock() const;
		
		void Unlock() const;
		
		bool IsLocked() const;
	private:
		mutable std::atomic_bool m_Locked = false;
	};

	class XYZ_API AssetRegistry
	{
	public:
		void StoreMetadata(const AssetMetadata& metadata, WeakRef<Asset> asset = nullptr);
		void RemoveMetadata(const AssetHandle& handle);
		void Clear();

		const AssetMetadata* GetMetadata(const AssetHandle& handle) const;
		const AssetMetadata* GetMetadata(const std::filesystem::path& filepath) const;
		const AssetMetadata* GetMetadata(const Ref<Asset>& asset) const { return GetMetadata(asset->GetHandle()); }

		Ref<AssetData> GetAsset(const AssetHandle& handle) const;
		Ref<AssetData> GetAsset(const std::filesystem::path& filepath) const;

		std::unordered_map<AssetHandle, Ref<AssetData>>::iterator		find(const AssetHandle& handle) { return m_Assets.find(handle); }
		std::unordered_map<AssetHandle, Ref<AssetData>>::const_iterator	find(const AssetHandle& handle) const { return m_Assets.find(handle); }

		std::unordered_map<AssetHandle, Ref<AssetData>>::iterator		begin()		  { return m_Assets.begin(); }
		std::unordered_map<AssetHandle, Ref<AssetData>>::iterator		end()		  { return m_Assets.end(); }
		std::unordered_map<AssetHandle, Ref<AssetData>>::const_iterator cbegin() const { return m_Assets.cbegin(); }
		std::unordered_map<AssetHandle, Ref<AssetData>>::const_iterator cend()	const { return m_Assets.cend(); }
	private:
		std::unordered_map<AssetHandle, Ref<AssetData>>					   m_Assets;
		std::unordered_map<std::filesystem::path, AssetHandle, PathHasher> m_AssetHandleMap;
	};
}