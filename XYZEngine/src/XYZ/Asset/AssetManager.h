#pragma once
#include "XYZ/Core/GUID.h"
#include "XYZ/Utils/DataStructures/MemoryPool.h"

#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Utils/FileSystem.h"

#include "XYZ/Renderer/Shader.h"
#include "XYZ/Renderer/Texture.h"
#include "XYZ/Renderer/SkeletalMesh.h"
#include "XYZ/FileWatcher/FileWatcher.h"

#include "AssetSerializer.h"
#include "AssetImporter.h"
#include "AssetFileListener.h"
#include "Asset.h"

namespace std {
	template <>
	struct hash<std::filesystem::path>
	{
		std::size_t operator()(const std::filesystem::path& path) const
		{
			return hash_value(path);
		}
	};
}

namespace XYZ {

	namespace Utils {

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

	namespace Editor {
		class AssetManagerViewPanel;
		class AssetBrowser;
	}

	class AssetManager
	{
	public:
		static void Init();
		static void Shutdown();

	
		template<typename T, typename... Args>
		static Ref<T> CreateAsset(const std::string& filename, const std::string& directoryPath, Args&&... args);
			
		template<typename T>
		static Ref<T> GetAsset(const GUID& assetHandle);

		template <typename T>
		static Ref<T> GetAsset(const std::filesystem::path& filepath);
		
		static void ReloadAsset(const std::filesystem::path& filepath);

		static const AssetMetadata& GetMetadata(const GUID& handle);
		static const AssetMetadata& GetMetadata(const std::filesystem::path& filepath);
		static const AssetMetadata& GetMetadata(const Ref<Asset>& asset) { return GetMetadata(asset->m_Handle); }
		
		static const std::string&	GetDirectory();
	private:
		static AssetMetadata& getMetadata(const GUID& handle);

		static void loadAssetMetadata(const std::filesystem::path& filepath);
		static void writeAssetMetadata(const AssetMetadata& metadata);

		static void processDirectory(const std::filesystem::path& path);
		static void reloadAsset(const AssetMetadata& metadata, Ref<Asset>& asset);
	private:
		static MemoryPool<1024 * 1024, true>					s_Pool;
		static std::unordered_map<GUID, WeakRef<Asset>>			s_LoadedAssets;
		static std::unordered_map<GUID, AssetMetadata>			s_AssetMetadata;
		static std::unordered_map<std::filesystem::path, GUID>	s_AssetHandleMap;
		
		static std::shared_ptr<FileWatcher>						s_FileWatcher;
		static AssetFileListener*								s_FileListener;
	private:
		friend Editor::AssetBrowser;
		friend Editor::AssetManagerViewPanel;
	};
	
	
	template<typename T, typename ...Args>
	inline Ref<T> AssetManager::CreateAsset(const std::string& filename, const std::string& directoryPath, Args && ...args)
	{
		static_assert(std::is_base_of<Asset, T>::value, "CreateAsset only works for types derived from Asset");
		AssetMetadata metadata;
		metadata.FilePath	  = directoryPath + "/" + filename;
		metadata.Type		  = T::GetStaticType();

		XYZ_ASSERT(FileSystem::Exists(metadata.FilePath), "File already exists");
		s_AssetMetadata[metadata.FilePath] = metadata;

		Ref<T> asset = Utils::CreateRef<T>(std::forward<Args>(args)...);
		asset->m_Handle = metadata.Handle;

		s_LoadedAssets[asset->m_Handle] = asset;
		AssetSerializer::SerializeAsset(asset);
		return asset;
	}


	template<typename T>
	inline Ref<T> AssetManager::GetAsset(const GUID& assetHandle)
	{	
		Ref<Asset> asset = nullptr;
		if (!s_LoadedAssets[assetHandle].IsValid())
		{
			auto& metadata = getMetadata(assetHandle);
			bool loaded = AssetImporter::TryLoadData(metadata, asset);
			if (!loaded)
				return nullptr;

			s_LoadedAssets[assetHandle] = asset;
		}
		else
		{
			asset = s_LoadedAssets[assetHandle].Raw();
		}
		return asset.As<T>();
	}
	template<typename T>
	inline Ref<T> AssetManager::GetAsset(const std::filesystem::path& filepath)
	{
		auto& metadata = GetMetadata(filepath);
		return GetAsset<T>(metadata.Handle);
	}
}