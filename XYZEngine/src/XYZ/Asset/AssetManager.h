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

	using AssetLoadedFn = std::function<void(Ref<Asset>)>;
	
	struct AssetPending
	{		
		Ref<AssetData>	AssetData;
		AssetLoadedFn	OnLoaded;
	};

	struct AssetLoaded
	{
		Ref<Asset> Asset;
		AssetLoadedFn OnLoaded;
	};

	class XYZ_API AssetManager
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
			
		static void LoadAssetAsync(const AssetHandle& assetHandle, const AssetLoadedFn& onLoaded);
		
		template<typename T>
		static Ref<T> GetAsset(const AssetHandle& assetHandle);

		template<typename T>
		static Ref<T> GetAssetWait(const AssetHandle& assetHandle);

		template <typename T>
		static Ref<T> GetAsset(const std::filesystem::path& filepath);

		template <typename T>
		static Ref<T> GetAssetWait(const std::filesystem::path& filepath);

		template<typename T>
		static Ref<T> TryGetAsset(const AssetHandle& assetHandle);

		template <typename T>
		static Ref<T> TryGetAsset(const std::filesystem::path& filepath);
		
		template <typename T>
		static std::vector<Ref<T>> FindAllAssets(AssetType type);

		template <typename T>
		static std::vector<Ref<T>> FindAllLoadedAssets(AssetType type);

		static std::vector<AssetHandle> FindAllLoadedAssets();

		static std::vector<AssetMetadata> FindAllMetadata(AssetType type);

		static void ReloadAsset(const std::filesystem::path& filepath);

		static const AssetMetadata GetMetadata(const AssetHandle& handle);
		static const AssetMetadata GetMetadata(const std::filesystem::path& filepath);
		static const AssetMetadata GetMetadata(const Ref<Asset>& asset) { return GetMetadata(asset->m_Handle); }
		
		static const std::filesystem::path&	GetAssetDirectory();
		static const MemoryPool&			GetMemoryPool() { return Get().m_Pool; }

		static bool Exist(const AssetHandle& handle);
		static bool Exist(const std::filesystem::path& filepath);

		static std::shared_ptr<FileWatcher> GetFileWatcher() { return Get().m_FileWatcher; }

		static AssetManager& Get();

	private:
		static void loadAssetMetadata(const std::filesystem::path& filepath);
		static void writeAssetMetadata(const AssetMetadata& metadata);
		static void processDirectory(const std::filesystem::path& path);
		static void keepAliveAsset(Ref<Asset> asset);

		template <typename T>
		static Ref<T> loadAsset(const AssetMetadata& metadata);


		static void onFileChange(FileWatcher::ChangeType type, const std::filesystem::path& path);

		static void assetLoadingWorker();
	private:
		MemoryPool										  m_Pool = MemoryPool(1024 * 1024 * 10);
		AssetRegistry									  m_Registry;

		std::unordered_map<AssetHandle, WeakRef<Asset>>   m_MemoryAssets;

		ThreadQueue<AssetPending>						  m_PendingAssetsQueue;
		ThreadQueue<AssetLoaded>						  m_LoadedAssetsQueue;

		std::unique_ptr<std::thread>					  m_PendingAssetsLoadThread;

		std::shared_ptr<FileWatcher>					  m_FileWatcher;
		std::shared_ptr<AssetLifeManager>				  m_AssetLifeManager;

		bool m_LoadingAssetsAsync;
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

		Get().m_MemoryAssets.[asset->m_Handle] = asset.Raw();
		return asset;
	}

	template<typename T>
	inline Ref<T> AssetManager::GetMemoryAsset(const AssetHandle& assetHandle)
	{
		WeakRef<Asset> asset;

		auto it = Get().m_MemoryAssets.find(assetHandle);
		XYZ_ASSERT(it != Get().m_MemoryAssets.end(), "Memory asset does not exist");
		return it->second.As<T>();
	}

	template<typename T, typename ...Args>
	inline Ref<T> AssetManager::CreateAsset(const std::string& filename, const std::string& directoryPath, Args && ...args)
	{
		static_assert(std::is_base_of<Asset, T>::value, "CreateAsset only works for types derived from Asset");
		AssetMetadata metadata;
		metadata.FilePath	  = directoryPath + "/" + filename;
		metadata.Type		  = T::GetStaticType();

		XYZ_ASSERT(!FileSystem::Exists(metadata.FilePath.string()), "File already exists");
		Ref<T> asset = Utils::CreateRef<T>(std::forward<Args>(args)...);
		asset->m_Handle = metadata.Handle;

		Get().m_Registry.StoreMetadata(metadata, asset.Raw());

		writeAssetMetadata(metadata);
		AssetImporter::Serialize(asset);
		keepAliveAsset(asset);
		return asset;
	}


	template<typename T>
	inline Ref<T> AssetManager::GetAsset(const AssetHandle& assetHandle)
	{
		Ref<Asset> result;
		Ref<AssetData> assetData = Get().m_Registry.GetAsset(assetHandle);
		if (assetData->TryLock()) // If asset is locked it means it is not loaded yet
		{
			result = assetData->Asset.Raw();
			if (!result.Raw())
			{		
				// If we did not request async load of asset, load it synchronously
				result = loadAsset<T>(assetData->Metadata);
				assetData->Asset = result.Raw();
			}
			assetData->Unlock();
		}
		return result.As<T>();
	}

	template<typename T>
	inline Ref<T> AssetManager::GetAssetWait(const AssetHandle& assetHandle)
	{
		Ref<AssetData> assetData = Get().m_Registry.GetAsset(assetHandle);
		assetData->SpinLock();

		Ref<Asset> result = assetData->Asset.Raw();
		if (!result.Raw())
		{
			// If we did not request async load of asset, load it synchronously
			result = loadAsset<T>(assetData->Metadata);
			assetData->Asset = result.Raw();
		}
		assetData->Unlock();
		return result.As<T>();
	}


	template<typename T>
	inline Ref<T> AssetManager::GetAsset(const std::filesystem::path& filepath)
	{
		auto metadata = Get().m_Registry.GetMetadata(filepath);
		return GetAsset<T>(metadata->Handle);
	}

	template<typename T>
	inline Ref<T> AssetManager::GetAssetWait(const std::filesystem::path& filepath)
	{
		auto metadata = Get().m_Registry.GetMetadata(filepath);
		return GetAssetWait<T>(metadata->Handle);
	}


	template<typename T>
	inline Ref<T> AssetManager::TryGetAsset(const AssetHandle& assetHandle)
	{
		auto metadata = Get().m_Registry.GetMetadata(assetHandle);
		if (metadata == nullptr)
			return Ref<T>();

		return GetAsset<T>(metadata->Handle);
	}
	
	template<typename T>
	inline Ref<T> AssetManager::TryGetAsset(const std::filesystem::path& filepath)
	{
		auto metadata = Get().m_Registry.GetMetadata(filepath);
		if (metadata == nullptr)
			return Ref<T>();

		return GetAsset<T>(metadata->Handle);
	}



	template<typename T>
	inline std::vector<Ref<T>> AssetManager::FindAllAssets(AssetType type)
	{
		std::vector<Ref<T>> result;
		for (auto it : Get().m_Registry)
		{
			const auto& metadata = it.second->Metadata;
			if (metadata.Type == type)
			{
				result.push_back(GetAsset<T>(metadata.Handle));
			}
		}
		return result;
	}

	template<typename T>
	inline std::vector<Ref<T>> AssetManager::FindAllLoadedAssets(AssetType type)
	{
		std::vector<Ref<T>> result;
		
		for (auto it : Get().m_Registry)
		{
			if (it.second->Asset.IsValid())
			{
				if (it.second->Metadata.Type == type)
				{
					Ref<Asset> asset = it.second->Asset.Raw();
					result.push_back(asset.As<T>());
				}
			}
		}
		return result;
	}

	template<typename T>
	inline Ref<T> AssetManager::loadAsset(const AssetMetadata& metadata)
	{
		Ref<Asset> result = nullptr;
		bool loaded = AssetImporter::TryLoadData(metadata, result);
		if (!loaded)
			return nullptr;

		keepAliveAsset(result);
		return result.As<T>();
	}
}