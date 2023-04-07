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
		s_Instance.m_LoadedAssetsQueue.Clear();
		s_Instance.m_PendingAssetsQueue.Clear();

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
		for (auto it : s_Instance.m_Registry)
		{
			if (it.second->Asset.IsValid())
			{
				const auto& metadata = it.second->Metadata;
				AssetImporter::Serialize(metadata, it.second->Asset);
			}
		}
	}

	void AssetManager::Serialize(const AssetHandle& assetHandle)
	{
		WeakRef<Asset> asset;
		auto it = s_Instance.m_Registry.find(assetHandle);
		if (it != s_Instance.m_Registry.end() && it->second->Asset.IsValid())
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
		while (!s_Instance.m_LoadedAssetsQueue.Empty())
		{
			AssetLoaded loaded = s_Instance.m_LoadedAssetsQueue.PopBack();
			loaded.OnLoaded(loaded.Asset);
		}
	}

	void AssetManager::LoadAssetAsync(const AssetHandle& assetHandle, const AssetLoadedFn& onLoaded)
	{
		Ref<AssetData> assetData = s_Instance.m_Registry.GetAsset(assetHandle);
		assetData->SpinLock(); // Wait to finish loading if it already started

		if (assetData->Asset.IsValid()) // Asset is already loaded just call callback immediately
		{
			Ref<Asset> asset = assetData->Asset.Raw();
			onLoaded(asset);
		}
		else
		{
			s_Instance.m_PendingAssetsQueue.PushBack({ assetData, onLoaded });
		}
		assetData->Unlock();
	}

	std::vector<AssetHandle> AssetManager::FindAllLoadedAssets()
	{
		std::vector<AssetHandle> result;

		for (const auto&[handle, assetData] : s_Instance.m_Registry)
		{
			if (assetData->Asset.IsValid())
				result.push_back(handle);
		}

		return result;
	}

	std::vector<AssetMetadata> AssetManager::FindAllMetadata(AssetType type)
	{
		std::vector<AssetMetadata> result;

		for (auto it : s_Instance.m_Registry)
		{
			const auto& metadata = it.second->Metadata;
			if (metadata.Type == type)
			{
				result.push_back(metadata);
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

			Ref<AssetData> assetData = s_Instance.m_Registry.GetAsset(metadata->Handle);
			assetData->WaitUnlock();

			if (assetData->Asset.IsValid())
			{
				assetData->Asset->SetFlag(AssetFlag::Reloaded);
			}
			assetData->Asset = asset;
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
			while (!s_Instance.m_PendingAssetsQueue.Empty())
			{
				AssetPending pending = s_Instance.m_PendingAssetsQueue.PopBack();
				pending.AssetData->SpinLock(); // Wait to be able to lock asset data again
				
				Ref<Asset> result;
				if (pending.AssetData->Asset.IsValid()) // Probably loaded asset on main thread already
				{
					result = pending.AssetData->Asset.Raw();
					s_Instance.m_LoadedAssetsQueue.PushBack({ result, std::move(pending.OnLoaded) });
				}
				else if (AssetImporter::TryLoadData(pending.AssetData->Metadata, result))
				{
					pending.AssetData->Asset = result;
					s_Instance.m_LoadedAssetsQueue.PushBack({ result, std::move(pending.OnLoaded) });
					keepAliveAsset(result);
				}
				pending.AssetData->Unlock();
			
			}
		}
	}

}