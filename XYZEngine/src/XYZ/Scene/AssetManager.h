#pragma once
#include "XYZ/Core/Ref.h"
#include "Serializer.h"

#include "XYZ/FileWatcher/FileWatcher.h"


namespace XYZ {
	
	//class IAsset
	//{
	//public:
	//	virtual ~IAsset() = default;
	//	virtual void Serialize() = 0;
	//};
	//
	//template <typename T>
	//class Asset : public IAsset
	//{
	//public:
	//	Asset(Ref<T> handle)
	//		: m_Handle(handle)
	//	{
	//		static_assert(std::is_base_of<Serializable, T>::value, "Class is not serializable!");
	//	}
	//	virtual void Serialize() override
	//	{
	//		Serializer::SerializeResource<T>(m_Handle);
	//	}
	//
	//	Ref<T> GetHandle() { return m_Handle; }
	//	const Ref<T> GetHandle() const { return m_Handle; }
	//
	//private:	
	//	Ref<T> m_Handle;
	//
	//	friend class AssetManager;
	//};

	//class AssetManager : public IFileWatcherListener
	//{
	//public:
	//	AssetManager(const std::string& directory);
	//	~AssetManager();
	//
	//	template <typename T>
	//	Asset<T>* GetAsset(const std::string& filepath)
	//	{
	//		auto it = m_Assets.find(filepath);
	//		if (it != m_Assets.end())
	//		{
	//			Asset<T>* casted = (Asset<T>*)it->second;
	//			if (casted->GetHandle().Raw())
	//				return casted;
	//
	//			Ref<T> ref = Serializer::DeserializeResource<T>(filepath, *this);
	//			casted->m_Handle = ref;
	//			casted;
	//		}
	//		Ref<T> ref = Serializer::DeserializeResource<T>(filepath, *this);
	//		IAsset* asset = new Asset<T>(ref);
	//		m_Assets[filepath] = asset;
	//		return (Asset<T>*)asset;
	//	}
	//
	//	void Serialize();
	//
	//protected:
	//	virtual void OnFileChange(const std::wstring& filepath) override;
	//	virtual void OnFileAdded(const std::wstring& filepath) override;
	//	virtual void OnFileRemoved(const std::wstring& filepath) override;
	//	virtual void OnFileRenamed(const std::wstring& filepath) override;
	//
	//private:
	//	std::unordered_map<std::string, IAsset*> m_Assets;
	//	std::shared_ptr<FileWatcher> m_FileWatcher;
	//	std::string m_Directory;
	//};
}