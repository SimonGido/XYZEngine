#pragma once
#include "XYZ/Core/GUID.h"
#include "XYZ/Core/Timestep.h"
#include "XYZ/Core/Application.h"

#include "XYZ/Utils/DataStructures/MemoryPool.h"
#include "XYZ/Utils/DataStructures/ThreadQueue.h"
#include "XYZ/Utils/DataStructures/ThreadUnorderedMap.h"

#include "XYZ/Utils/StringUtils.h"
#include "XYZ/Utils/FileSystem.h"

#include "XYZ/Renderer/Shader.h"
#include "XYZ/Renderer/Texture.h"
#include "XYZ/Scene/Prefab.h"

#include "XYZ/FileWatcher/FileWatcher.h"

#include "AssetSerializer.h"
#include "AssetImporter.h"
#include "AssetRegistry.h"
#include "AssetLifeManager.h"
#include "Asset.h"


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
		static void KeepAlive(float seconds);


		static void SerializeAll();
		static void Serialize(const AssetHandle& assetHandle);

		static void Update(Timestep ts);

		template<typename T, typename... Args>
		static Ref<T> CreateMemoryAsset(const std::string& name, Args&&... args);

		template<typename T>
		static Ref<T> GetMemoryAsset(const AssetHandle& assetHandle);

		template<typename T, typename... Args>
		static Ref<T> CreateAsset(const std::string& filename, const std::string& directoryPath, Args&&... args);
			
		template<typename T>
		static Ref<T> GetAsset(const AssetHandle& assetHandle);

		template <typename T>
		static Ref<T> GetAsset(const std::filesystem::path& filepath);

		template <typename T>
		static std::future<Ref<T>> GetAssetAsync(const std::filesystem::path& filepath);

		template<typename T>
		static Ref<T> TryGetAsset(const AssetHandle& assetHandle);

		template <typename T>
		static Ref<T> TryGetAsset(const std::filesystem::path& filepath);
		
		template <typename T>
		static std::vector<Ref<T>> FindAllAssets(AssetType type);

		template <typename T>
		static std::vector<Ref<T>> FindAllLoadedAssets(AssetType type);

		static std::vector<AssetMetadata> FindAllMetadata(AssetType type);

		static void ReloadAsset(const std::filesystem::path& filepath);

		static const AssetMetadata& GetMetadata(const AssetHandle& handle);
		static const AssetMetadata& GetMetadata(const std::filesystem::path& filepath);
		static const AssetMetadata& GetMetadata(const Ref<Asset>& asset) { return GetMetadata(asset->m_Handle); }
		
		static const std::filesystem::path&	GetAssetDirectory();
		static const MemoryPool&			GetMemoryPool() { return s_Pool; }

		static bool Exist(const AssetHandle& handle);
		static bool Exist(const std::filesystem::path& filepath);

		static std::shared_ptr<FileWatcher> GetFileWatcher() { return s_FileWatcher; }	

	private:
		static void loadAssetMetadata(const std::filesystem::path& filepath);
		static void writeAssetMetadata(const AssetMetadata& metadata);

		static void processDirectory(const std::filesystem::path& path);

		static void onFileChange(FileWatcher::ChangeType type, const std::filesystem::path& path);

	private:
		static MemoryPool											  s_Pool;
		static AssetRegistry										  s_Registry;
		static ThreadUnorderedMap<AssetHandle, WeakRef<Asset>>		  s_LoadedAssets;
		static ThreadUnorderedMap<AssetHandle, WeakRef<Asset>>        s_MemoryAssets;

		static std::shared_ptr<FileWatcher>							  s_FileWatcher;
		static std::shared_ptr<AssetLifeManager>					  s_AssetLifeManager;

	private:
		friend Editor::AssetBrowser;
		friend Editor::AssetManagerViewPanel;
		friend class   AssetLifeManager;
	};
	
	
	template<typename T, typename ...Args>
	inline Ref<T> AssetManager::CreateMemoryAsset(const std::string& name, Args && ...args)
	{
		static_assert(std::is_base_of<Asset, T>::value, "CreateAsset only works for types derived from Asset");

		Ref<T> asset = Utils::CreateRef<T>(std::forward<Args>(args)...);
		asset->m_Handle = AssetHandle();

		s_MemoryAssets.Set(asset->m_Handle, asset.Raw());
		return asset;
	}

	template<typename T>
	inline Ref<T> AssetManager::GetMemoryAsset(const AssetHandle& assetHandle)
	{
		WeakRef<Asset> asset;
		bool found = s_MemoryAssets.Find(assetHandle, asset);
		XYZ_ASSERT(!found, "Memory asset does not exist");
		return asset.As<T>();
	}

	template<typename T, typename ...Args>
	inline Ref<T> AssetManager::CreateAsset(const std::string& filename, const std::string& directoryPath, Args && ...args)
	{
		static_assert(std::is_base_of<Asset, T>::value, "CreateAsset only works for types derived from Asset");
		AssetMetadata metadata;
		metadata.FilePath	  = directoryPath + "/" + filename;
		metadata.Type		  = T::GetStaticType();

		XYZ_ASSERT(!FileSystem::Exists(metadata.FilePath.string()), "File already exists");
		
		s_Registry.StoreMetadata(metadata);

		Ref<T> asset = Utils::CreateRef<T>(std::forward<Args>(args)...);
		asset->m_Handle = metadata.Handle;

		s_LoadedAssets.Set(asset->m_Handle, asset.Raw());
		writeAssetMetadata(metadata);
		AssetImporter::Serialize(asset);
		return asset;
	}


	template<typename T>
	inline Ref<T> AssetManager::GetAsset(const AssetHandle& assetHandle)
	{
		Ref<Asset> result = nullptr;
		WeakRef<Asset> getAsset = nullptr;
		if (!s_LoadedAssets.TryGet(assetHandle, getAsset) || !getAsset.IsValid())
		{
			auto metadata = s_Registry.GetMetadata(assetHandle);
			bool loaded = AssetImporter::TryLoadData(*metadata, result);
			if (!loaded)
				return nullptr;

			s_LoadedAssets.Set(assetHandle, result.Raw());
			return result;
		}

		result = getAsset.Raw();
		return result.As<T>();
	}


	template<typename T>
	inline Ref<T> AssetManager::GetAsset(const std::filesystem::path& filepath)
	{
		auto& metadata = GetMetadata(filepath);
		return GetAsset<T>(metadata.Handle);
	}

	template<typename T>
	inline std::future<Ref<T>> AssetManager::GetAssetAsync(const std::filesystem::path& filepath)
	{
		auto& threadPool = Application::Get().GetThreadPool();
		return threadPool.SubmitJob([path = filepath]() {
			return GetAsset<T>(path);
		});
	}


	template<typename T>
	inline Ref<T> AssetManager::TryGetAsset(const AssetHandle& assetHandle)
	{
		auto metadata = s_Registry.GetMetadata(assetHandle);
		if (metadata == nullptr)
			return Ref<T>();

		return GetAsset<T>(metadata->Handle);
	}
	
	template<typename T>
	inline Ref<T> AssetManager::TryGetAsset(const std::filesystem::path& filepath)
	{
		auto metadata = s_Registry.GetMetadata(filepath);
		if (metadata == nullptr)
			return Ref<T>();

		return GetAsset<T>(metadata->Handle);
	}



	template<typename T>
	inline std::vector<Ref<T>> AssetManager::FindAllAssets(AssetType type)
	{
		std::vector<Ref<T>> result;

		s_LoadedAssets.ForEach([&result, type](const AssetHandle& handle, WeakRef<Asset> asset) {
			auto metadata = s_Registry.GetMetadata(handle);
			if (metadata->Type == type)
			{
				result.push_back(GetAsset<T>(handle));
			}
		});
		return result;
	}

	template<typename T>
	inline std::vector<Ref<T>> AssetManager::FindAllLoadedAssets(AssetType type)
	{
		std::vector<Ref<T>> result;

		s_LoadedAssets.ForEach([&result, type](const AssetHandle& handle, WeakRef<Asset> asset) {
			auto metadata = s_Registry.GetMetadata(handle);
			if (metadata->Type == type)
			{
				WeakRef<Asset> getAsset = nullptr;
				if (s_LoadedAssets.TryGet(metadata->Handle, getAsset) && getAsset.IsValid())
				{
					Ref<T> asset = getAsset.As<T>().Raw();
					result.push_back(asset);
				}
			}
			});
		return result;
	}
}