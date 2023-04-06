#include "stdafx.h"
#include "AssetManager.h"
#include "XYZ/Project/Project.h"

#include <filesystem>

#include <yaml-cpp/yaml.h>


namespace XYZ
{
	static std::filesystem::path s_Directory = "Assets";
	static AssetManager s_Instance;

	void AssetManager::Init()
	{
		AssetImporter::Init();
		processDirectory(s_Directory);
		processDirectory("Resources");

		std::wstring wdir = s_Directory.wstring();
		s_Instance.m_FileWatcher = std::make_shared<FileWatcher>(wdir);
		
		s_Instance.m_FileWatcher->AddOnFileChanged<&onFileChange>();
		s_Instance.m_FileWatcher->Start();

		s_Instance.m_LoadingAssetsAsync = true;
		s_Instance.m_PendingAssetsLoadThread = std::make_unique<std::thread>(&AssetManager::assetLoadingWorker);
	}
	void AssetManager::Shutdown()
	{
		s_Instance.m_LoadingAssetsAsync = false;
		s_Instance.m_PendingAssetsLoadThread->join();
		s_Instance.m_PendingAssetsLoadThread.reset();

		s_Instance.m_LoadedAssets.clear();
		s_Instance.m_MemoryAssets.clear();
		s_Instance.m_Registry.Clear();

		s_Instance.m_FileWatcher->Stop();
		if (s_Instance.m_AssetLifeManager)
			s_Instance.m_AssetLifeManager->Stop();
	}

	void AssetManager::KeepAlive(float seconds)
	{
		if (!s_Instance.m_AssetLifeManager)
			s_Instance.m_AssetLifeManager = std::make_shared<AssetLifeManager>();

		s_Instance.m_AssetLifeManager->Start(seconds);
	}



	void AssetManager::SerializeAll()
	{
		for (auto it : s_Instance.m_LoadedAssets)
		{
			if (it.second.IsValid())
			{
				const auto metadata = s_Instance.m_Registry.GetMetadata(it.first);
				AssetImporter::Serialize(*metadata, it.second);
			}
		}
	}

	void AssetManager::Serialize(const AssetHandle& assetHandle)
	{
		WeakRef<Asset> asset;
		auto it = s_Instance.m_LoadedAssets.find(assetHandle);
		if (it != s_Instance.m_LoadedAssets.end() && it->second.IsValid())
		{
			const auto metadata = s_Instance.m_Registry.GetMetadata(assetHandle);
			AssetImporter::Serialize(*metadata, asset);
		}
		else
		{
			XYZ_CORE_WARN("Trying to serialize asset that does not exist!");
		}
	}

	void AssetManager::Update(Timestep ts)
	{
		s_Instance.m_FileWatcher->ProcessChanges();
	}

	void AssetManager::LoadAssetAsync(const AssetHandle& assetHandle, const AssetLoadContext::AssetLoadedFn& onLoaded)
	{
		Ref<AssetPtr> assetPtr = s_Instance.m_Registry.GetAsset(assetHandle);
		if (assetPtr->Pending)
			return;
		
		if (assetPtr->Asset.IsValid())
		{
			Ref<Asset> asset = assetPtr->Asset.Raw();
			onLoaded(asset);
			return;
		}

		assetPtr->Pending = true;
		s_Instance.m_PendingAssets.PushBack({ assetPtr, onLoaded });
	}

	std::vector<AssetHandle> AssetManager::FindAllLoadedAssets()
	{
		std::vector<AssetHandle> result;

		for (const auto&[handle, asset] : s_Instance.m_LoadedAssets)
		{
			if (asset.IsValid())
				result.push_back(handle);
		}

		return result;
	}

	std::vector<AssetMetadata> AssetManager::FindAllMetadata(AssetType type)
	{
		std::vector<AssetMetadata> result;

		for (auto it : s_Instance.m_LoadedAssets)
		{
			auto metadata = s_Instance.m_Registry.GetMetadata(it.first);
			if (metadata->Type == type)
			{
				result.push_back(*metadata);
			}
		}

		return result;
	}
	void AssetManager::ReloadAsset(const std::filesystem::path& filepath)
	{
		const auto metadata = s_Instance.m_Registry.GetMetadata(filepath);
		if (metadata)
		{
			Ref<Asset> asset = nullptr;
			bool loaded = AssetImporter::TryLoadData(*metadata, asset);
			if (!loaded)
			{
				XYZ_CORE_WARN("Could not load asset {}", filepath);
				return;
			}

			WeakRef<Asset> weakAsset = findAsset(metadata->Handle);
			if (weakAsset.IsValid())
			{
				weakAsset->SetFlag(AssetFlag::Reloaded);
			}

			s_Instance.m_LoadedAssets[asset->GetHandle()] = asset;
		}
	}

	const AssetMetadata AssetManager::GetMetadata(const AssetHandle& handle)
	{
		auto metadata = s_Instance.m_Registry.GetMetadata(handle);
		XYZ_ASSERT(metadata, "Metadata does not exist");
		return *metadata;
	}

	const AssetMetadata AssetManager::GetMetadata(const std::filesystem::path& filepath)
	{
		auto metadata = s_Instance.m_Registry.GetMetadata(filepath);
		XYZ_ASSERT(metadata, "Metadata does not exist");
		return *metadata;
	}

	const std::filesystem::path& AssetManager::GetAssetDirectory()
	{
		return s_Directory;
	}
	
	bool AssetManager::Exist(const AssetHandle& handle)
	{
		return s_Instance.m_Registry.GetMetadata(handle) != nullptr;
	}

	bool AssetManager::Exist(const std::filesystem::path& filepath)
	{
		return s_Instance.m_Registry.GetMetadata(filepath) != nullptr;
	}

	bool AssetManager::IsAssetLoaded(const AssetHandle& handle)
	{
		WeakRef<Asset> getAsset = findAsset(handle);
		return getAsset.IsValid();
	}

	AssetManager& AssetManager::Get()
	{
		return s_Instance;
	}

	void AssetManager::loadAssetMetadata(const std::filesystem::path& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());

		auto handle = data["Handle"];
		auto filePath = data["FilePath"];
		auto type = data["Type"];

		if (handle && filePath && type)
		{
			AssetHandle guid(handle.as<std::string>());
			AssetMetadata metadata;
			metadata.Handle = guid;
			metadata.FilePath = filePath.as<std::string>();
			metadata.Type = Utils::AssetTypeFromString(type.as<std::string>());
			s_Instance.m_Registry.StoreMetadata(metadata);
		}
		else
		{
			XYZ_CORE_WARN("Failed to load asset meta data {0}", filepath);
		}
	}

	void AssetManager::writeAssetMetadata(const AssetMetadata& metadata)
	{
		std::string filepath = metadata.FilePath.string();
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Handle" << YAML::Value << (std::string)metadata.Handle;
		out << YAML::Key << "FilePath" << YAML::Value << filepath;
		out << YAML::Key << "Type" << YAML::Value << Utils::AssetTypeToString(metadata.Type);
		out << YAML::EndMap;

		std::ofstream fout(filepath + ".meta");
		fout << out.c_str();
	}

	void AssetManager::processDirectory(const std::filesystem::path& path)
	{
		for (auto it : std::filesystem::directory_iterator(path))
		{
			if (it.is_directory())
			{
				processDirectory(it.path());
			}
			else if (Utils::GetExtension(it.path().string()) == "meta")
			{
				loadAssetMetadata(it.path());
			}
		}
	}

	void AssetManager::keepAliveAsset(Ref<Asset> asset)
	{
		if (s_Instance.m_AssetLifeManager)
			s_Instance.m_AssetLifeManager->PushAsset(asset);
	}

	WeakRef<Asset> AssetManager::findAsset(const AssetHandle& assetHandle)
	{
		WeakRef<Asset> getAsset = nullptr;

		auto it = s_Instance.m_LoadedAssets.find(assetHandle);
		if (it != s_Instance.m_LoadedAssets.end())
		{
			getAsset = it->second;
		}
		return getAsset;
	}


	static std::filesystem::path s_RenamedFileOldPath;

	void AssetManager::onFileChange(FileWatcher::ChangeType type, const std::filesystem::path& path)
	{
		if (type == FileWatcher::ChangeType::Modified)
		{
			if (AssetManager::Exist(path))
			{
				AssetManager::ReloadAsset(path);
			}
		}
		else if (type == FileWatcher::ChangeType::Added)
		{

		}
		else if (type == FileWatcher::ChangeType::Removed)
		{
			const auto metadata = s_Instance.m_Registry.GetMetadata(path);
			if (metadata)
			{
				s_Instance.m_Registry.RemoveMetadata((*metadata).Handle);
				s_Instance.m_LoadedAssets.erase((*metadata).Handle);
			}
		}
		else if (type == FileWatcher::ChangeType::RenamedOld)
		{
			s_RenamedFileOldPath = path;
		}
		else if (type == FileWatcher::ChangeType::RenamedNew)
		{
			const auto ptrMetadata = s_Instance.m_Registry.GetMetadata(s_RenamedFileOldPath);
			if (ptrMetadata)
			{
				auto metadata = *ptrMetadata;
				s_Instance.m_Registry.RemoveMetadata(metadata.Handle);

				FileSystem::Rename(s_RenamedFileOldPath.string() + ".meta", Utils::GetFilename(path.string()));

				metadata.FilePath = path;
				s_Instance.m_Registry.StoreMetadata(metadata);
				s_Instance.writeAssetMetadata(metadata);
			}
		}
	}

	void AssetManager::assetLoadingWorker()
	{
		while (s_Instance.m_LoadingAssetsAsync)
		{
			while (!s_Instance.m_PendingAssets.Empty())
			{
				AssetLoadContext loadContext = s_Instance.m_PendingAssets.PopFront();
				
				if (loadContext.AssetPtr->Pending)
				{
					Ref<Asset> result;
					if (AssetImporter::TryLoadData(loadContext.AssetPtr->Metadata, result))
					{
						loadContext.AssetPtr->Asset = result;
						loadContext.OnLoaded(result);
						keepAliveAsset(result);
					}
					loadContext.AssetPtr->Pending = false;
				}
			}
		}
	}

}