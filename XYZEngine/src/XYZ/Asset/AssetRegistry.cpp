#include "stdafx.h"
#include "AssetRegistry.h"


namespace XYZ {
	void AssetRegistry::StoreMetadata(const AssetMetadata& metadata)
	{
		m_AssetMetadata[metadata.Handle] = metadata;
		m_AssetHandleMap[metadata.FilePath] = metadata.Handle;
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