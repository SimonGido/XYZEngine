#pragma once
#include "XYZ/Core/Ref.h"
#include "Serializable.h"
#include "Serializer.h"

#include "XYZ/FileWatcher/FileWatcher.h"


namespace XYZ {
	

	class AssetManager : public IFileWatcherListener
	{
	public:
		AssetManager(const std::string& directory);
		~AssetManager();

		template <typename T>
		Ref<T> GetAsset(const std::string& filepath)
		{
			std::string fullPath = m_Directory + "\\" + filepath;
			static_assert(std::is_base_of<Serializable, T>::value, "Class is not serializable!");
			auto it = m_Assets.find(fullPath);
			if (it != m_Assets.end() && it->second->GetRefCount())
				return Ref<T>((T*)it->second);
			
			Ref<T> ref = Serializer::DeserializeResource<T>(fullPath);
			m_Assets[fullPath] = ref.Raw();
			return ref;
		}

		virtual void OnFileChange(const std::wstring& filepath) override;
		virtual void OnFileAdded(const std::wstring& filepath) override;
		virtual void OnFileRemoved(const std::wstring& filepath) override;
		virtual void OnFileRenamed(const std::wstring& filepath) override;

	private:
		std::unordered_map<std::string, RefCount*> m_Assets;
		std::unique_ptr<FileWatcher> m_FileWatcher;
		std::string m_Directory;
	};
}