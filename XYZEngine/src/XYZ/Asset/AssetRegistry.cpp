#include "stdafx.h"
#include "AssetRegistry.h"

#include "XYZ/Asset/AssetManager.h"

namespace XYZ {
	void AssetRegistry::StoreMetadata(const AssetMetadata& metadata, WeakRef<Asset> asset = nullptr)
	{
		Ref<AssetPtr> assetPtr = Ref<AssetPtr>::Create();
		assetPtr->Metadata = metadata;
		assetPtr->Asset = asset;

		m_Assets[metadata.Handle] = assetPtr;
		m_AssetHandleMap[metadata.FilePath] = metadata.Handle;
	}

	void AssetRegistry::RemoveMetadata(const AssetHandle& handle)
	{
		auto it = m_Assets.find(handle);
		XYZ_ASSERT(it != m_Assets.end(), "");
		m_AssetHandleMap.erase(it->second->Metadata.FilePath);
		m_Assets.erase(it);
	}

	void AssetRegistry::Clear()
	{
		m_Assets.clear();
	}
	
	const AssetMetadata* AssetRegistry::GetMetadata(const AssetHandle& handle) const
	{
		auto it = m_Assets.find(handle);
		if (it != m_Assets.end())
			return &it->second->Metadata;

		return nullptr;
	}
	const AssetMetadata* AssetRegistry::GetMetadata(const std::filesystem::path& filepath) const
	{
		auto it = m_AssetHandleMap.find(filepath);
		if (it != m_AssetHandleMap.end())
		{
			return &m_Assets.find(it->second)->second->Metadata;
		}
		return nullptr;
	}
	Ref<AssetPtr> AssetRegistry::GetAsset(const AssetHandle& handle) const
	{
		return m_Assets.at(handle);
	}
	Ref<AssetPtr> AssetRegistry::GetAsset(const std::filesystem::path& filepath) const
	{
		auto metadata = GetMetadata(filepath);
		return m_Assets.at(metadata->Handle);
	}
}