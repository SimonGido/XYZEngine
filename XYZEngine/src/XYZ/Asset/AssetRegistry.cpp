#include "stdafx.h"
#include "AssetRegistry.h"

#include "XYZ/Asset/AssetManager.h"

namespace XYZ {
	void AssetRegistry::StoreMetadata(const AssetMetadata& metadata)
	{
		m_AssetMetadata[metadata.Handle] = metadata;
		m_AssetHandleMap[metadata.FilePath] = metadata.Handle;
	}

	void AssetRegistry::RemoveMetadata(const AssetHandle& handle)
	{
		auto it = m_AssetMetadata.find(handle);
		XYZ_ASSERT(it != m_AssetMetadata.end(), "");
		m_AssetHandleMap.erase(it->second.FilePath);
		m_AssetMetadata.erase(it);
	}
	
	const AssetMetadata* AssetRegistry::GetMetadata(const AssetHandle& handle) const
	{
		auto it = m_AssetMetadata.find(handle);
		if (it != m_AssetMetadata.end())
			return &it->second;

		return nullptr;
	}
	const AssetMetadata* AssetRegistry::GetMetadata(const std::filesystem::path& filepath) const
	{
		auto it = m_AssetHandleMap.find(filepath);
		if (it != m_AssetHandleMap.end())
		{
			return &m_AssetMetadata.find(it->second)->second;
		}
		return nullptr;
	}
}