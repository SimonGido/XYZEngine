#pragma once
#include "XYZ/Core/GUID.h"
#include "XYZ/Utils/DataStructures/MemoryPool.h"

#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Renderer/Shader.h"
#include "XYZ/Renderer/Texture.h"
#include "XYZ/Renderer/SkeletalMesh.h"
#include "AssetSerializer.h"
#include "Asset.h"

namespace XYZ {

	namespace Helper {

		template <typename T, typename...Args>
		inline typename std::enable_if<
			(!std::is_same<T, Shader>::value) && 
			(!std::is_same<T, Texture2D>::value), 
			Ref<T>
		>::type
		CreateRef(Args&&... args)
		{
			return Ref<T>::Create(std::forward<Args>(args)...);
		}


		template <typename T, typename ...Args>
		inline typename std::enable_if<std::is_same<T, Shader>::value, Ref<T>>::type 
		CreateRef(Args&&... args)
		{
			return Shader::Create(std::forward<Args>(args)...);
		}

		template <typename T, typename ...Args>
		inline typename std::enable_if<std::is_same<T, Texture2D>::value, Ref<T>>::type 
			CreateRef(Args&&... args)
		{
			return Texture2D::Create(std::forward<Args>(args)...);
		}
	}

	class AssetManager
	{
	public:
		static void Init();
		static void Shutdown();


		static void		 DisplayMemory();
		static AssetType GetAssetTypeFromExtension(const std::string& extension);
		static GUID		 GetAssetHandle(const std::string& filepath);
		static GUID		 GetDirectoryHandle(const std::string& filepath);
		static std::vector<Ref<Asset>> FindAssetsByType(AssetType type);
		static bool	     IsValidExtension(const std::string& extension);
		static void      CreateDirectory(const std::string& dirName);
		
		template<typename T, typename... Args>
		static Ref<T> CreateAsset(const std::string& filename, const std::string& directoryPath, Args&&... args);
			
		template<typename T>
		static Ref<T> GetAsset(const GUID& assetHandle, bool loadData = true);
		
	private:
		static void processDirectory(const std::string& path, AssetDirectory& directory);
		static void importAsset(const std::string& path);
		

	private:
		static MemoryPool<1024 * 1024, true>			  s_Pool;
		static std::unordered_map<GUID, Ref<Asset>>       s_LoadedAssets;
		static std::unordered_map<GUID, AssetDirectory>   s_Directories;
		static std::unordered_map<std::string, AssetType> s_AssetTypes;
	};
	
	
	template<typename T, typename ...Args>
	inline Ref<T> AssetManager::CreateAsset(const std::string& filename, const std::string& directoryPath, Args && ...args)
	{
		static_assert(std::is_base_of<Asset, T>::value, "CreateAsset only works for types derived from Asset");

		const GUID directoryHandle = GetDirectoryHandle(directoryPath);
		auto& directory = s_Directories[directoryHandle];
		Ref<T> asset = Helper::CreateRef<T>(std::forward<Args>(args)...);
		asset->Type = T::GetStaticType();
		asset->FilePath = directory.FilePath + "/" + filename;
		asset->FileName = Utils::RemoveExtension(Utils::GetFilename(asset->FilePath));
		asset->FileExtension = Utils::GetFilename(filename);
		asset->DirectoryHandle = directoryHandle;
		asset->Handle = GUID();
		asset->IsLoaded = true;
		s_LoadedAssets[asset->Handle] = asset;
		AssetSerializer::SerializeAsset(asset);
		return asset;
	}


	template<typename T>
	inline Ref<T> AssetManager::GetAsset(const GUID& assetHandle, bool loadData)
	{
		XYZ_ASSERT(s_LoadedAssets.find(assetHandle) != s_LoadedAssets.end(), "");
		Ref<Asset> asset = s_LoadedAssets[assetHandle];

		if (!asset->IsLoaded && loadData)
		{
			asset = AssetSerializer::LoadAsset(asset);
			asset->IsLoaded = true;
			s_LoadedAssets[assetHandle] = asset;
		}
		return asset.As<T>();
	}
}