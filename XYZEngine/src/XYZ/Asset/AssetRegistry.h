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

	class AssetRegistry
	{
	public:
		void StoreMetadata(const AssetMetadata& metadata);
		void RemoveMetadata(const AssetHandle& handle);

		const AssetMetadata* GetMetadata(const AssetHandle& handle) const;
		const AssetMetadata* GetMetadata(const std::filesystem::path& filepath) const;
		const AssetMetadata* GetMetadata(const Ref<Asset>& asset) const { return GetMetadata(asset->GetHandle()); }

		std::unordered_map<AssetHandle, AssetMetadata>::iterator	   begin()		  { return m_AssetMetadata.begin(); }
		std::unordered_map<AssetHandle, AssetMetadata>::iterator	   end()		  { return m_AssetMetadata.end(); }
		std::unordered_map<AssetHandle, AssetMetadata>::const_iterator cbegin() const { return m_AssetMetadata.cbegin(); }
		std::unordered_map<AssetHandle, AssetMetadata>::const_iterator cend()	const { return m_AssetMetadata.cend(); }
	private:
		std::unordered_map<AssetHandle, AssetMetadata>					   m_AssetMetadata;
		std::unordered_map<std::filesystem::path, AssetHandle, PathHasher> m_AssetHandleMap;
	};
}