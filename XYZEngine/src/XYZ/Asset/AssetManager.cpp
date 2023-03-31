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
	}
	void AssetManager::Shutdown()
	{
		s_Instance.m_LoadedAssets.clear();
		s_Instance.m_MemoryAssets.clear();
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
		auto loadedAssets = getLoadedAssetsRead();
		auto registry = getRegistryRead();

		for (auto it : loadedAssets.As())
		{
			if (it.second.IsValid())
			{
				const auto metadata = registry->GetMetadata(it.first);
				AssetImporter::Serialize(*metadata, it.second);
			}
		}
	}

	void AssetManager::Serialize(const AssetHandle& assetHandle)
	{
		WeakRef<Asset> asset;
		auto loadedAssets = getLoadedAssetsRead();
		auto it = loadedAssets->find(assetHandle);
		if (it != loadedAssets->end() && it->second.IsValid())
		{
			auto registry = getRegistryRead();
			const auto metadata = registry->GetMetadata(assetHandle);
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


	std::vector<AssetHandle> AssetManager::FindAllLoadedAssets()
	{
		auto loadedAssets = getLoadedAssetsRead();
		std::vector<AssetHandle> result;

		for (const auto&[handle, asset] : loadedAssets.As())
		{
			if (asset.IsValid())
				result.push_back(handle);
		}

		return result;
	}

	std::vector<AssetMetadata> AssetManager::FindAllMetadata(AssetType type)
	{
		std::vector<AssetMetadata> result;
		auto registry = getRegistryRead();
		auto loadedAssets = getLoadedAssetsRead();

		for (auto it : loadedAssets.As())
		{
			auto metadata = registry->GetMetadata(it.first);
			if (metadata->Type == type)
			{
				result.push_back(*metadata);
			}
		}

		return result;
	}
	void AssetManager::ReloadAsset(const std::filesystem::path& filepath)
	{
		auto registry = getRegistryRead();

		const auto metadata = registry->GetMetadata(filepath);
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

			auto loadedAssets = getLoadedAssetsWrite();
			loadedAssets.As()[asset->GetHandle()] = asset;
		}
	}

	const AssetMetadata AssetManager::GetMetadata(const AssetHandle& handle)
	{
		auto registry = getRegistryRead();
		auto metadata = registry->GetMetadata(handle);
		XYZ_ASSERT(metadata, "Metadata does not exist");
		return *metadata;
	}

	const AssetMetadata AssetManager::GetMetadata(const std::filesystem::path& filepath)
	{
		auto registry = getRegistryRead();
		auto metadata = registry->GetMetadata(filepath);
		XYZ_ASSERT(metadata, "Metadata does not exist");
		return *metadata;
	}

	const std::filesystem::path& AssetManager::GetAssetDirectory()
	{
		return s_Directory;
	}
	
	bool AssetManager::Exist(const AssetHandle& handle)
	{
		auto registry = getRegistryRead();
		return registry->GetMetadata(handle) != nullptr;
	}

	bool AssetManager::Exist(const std::filesystem::path& filepath)
	{
		auto registry = getRegistryRead();
		return registry->GetMetadata(filepath) != nullptr;
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

	void AssetManager::tryKeepAliveAsset(Ref<Asset> asset)
	{
		if (s_Instance.m_AssetLifeManager)
			s_Instance.m_AssetLifeManager->PushAsset(asset);
	}

	WeakRef<Asset> AssetManager::findAsset(const AssetHandle& assetHandle)
	{
		WeakRef<Asset> getAsset = nullptr;

		auto loadedAssets = getLoadedAssetsRead();
		auto it = loadedAssets->find(assetHandle);
		if (it != loadedAssets->end())
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
			auto registry = getRegistryWrite();
			const auto metadata = registry->GetMetadata(path);
			if (metadata)
			{
				registry->RemoveMetadata((*metadata).Handle);
				auto loadedAssets = getLoadedAssetsWrite();
				loadedAssets->erase((*metadata).Handle);
			}
		}
		else if (type == FileWatcher::ChangeType::RenamedOld)
		{
			s_RenamedFileOldPath = path;
		}
		else if (type == FileWatcher::ChangeType::RenamedNew)
		{
			auto registry = getRegistryWrite();
			const auto ptrMetadata = registry->GetMetadata(s_RenamedFileOldPath);
			if (ptrMetadata)
			{
				auto metadata = *ptrMetadata;
				registry->RemoveMetadata(metadata.Handle);

				FileSystem::Rename(s_RenamedFileOldPath.string() + ".meta", Utils::GetFilename(path.string()));

				metadata.FilePath = path;
				registry->StoreMetadata(metadata);
				s_Instance.writeAssetMetadata(metadata);
			}
		}
	}
	ScopedLock<AssetManager::AssetStorage> AssetManager::getMemoryAssetsWrite()
	{
		return ScopedLock<AssetStorage>(&s_Instance.m_MemoryAssetsMutex, s_Instance.m_MemoryAssets);
	}
	ScopedLockRead<AssetManager::AssetStorage> AssetManager::getMemoryAssetsRead()
	{
		return ScopedLockRead<AssetStorage>(&s_Instance.m_MemoryAssetsMutex, s_Instance.m_MemoryAssets);
	}
	ScopedLock<AssetManager::AssetStorage> AssetManager::getLoadedAssetsWrite()
	{
		return ScopedLock<AssetStorage>(&s_Instance.m_AssetsMutex, s_Instance.m_LoadedAssets);
	}
	ScopedLockRead<AssetManager::AssetStorage> AssetManager::getLoadedAssetsRead()
	{
		return ScopedLockRead<AssetStorage>(&s_Instance.m_AssetsMutex, s_Instance.m_LoadedAssets);
	}
	ScopedLock<AssetRegistry> AssetManager::getRegistryWrite()
	{
		return ScopedLock<AssetRegistry>(&s_Instance.m_RegistryMutex, s_Instance.m_Registry);
	}
	ScopedLockRead<AssetRegistry> AssetManager::getRegistryRead()
	{
		return ScopedLockRead<AssetRegistry>(&s_Instance.m_RegistryMutex, s_Instance.m_Registry);
	}
}