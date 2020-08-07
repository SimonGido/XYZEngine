#pragma once
#include "XYZ/Core/Ref.h"


namespace XYZ {
	
	template <typename T>
	struct Asset
	{
		friend class AssetManager;

		std::string Filepath;
		Ref<T> Handle;
		

		void Serialize();

		void Deserialize(AssetManager& manager);
	};

	class IAssetList
	{
	public:
		virtual ~IAssetList() {};

		virtual void Serialize() = 0;

	protected:
		virtual size_t GetSize() const = 0;
	};

	template <typename T>
	class AssetList : public IAssetList
	{
		friend class AssetManager;
	public:

		virtual void Serialize() override
		{
			for (auto& asset : m_Assets)
				asset.second.Serialize();		
		}

		void Insert(const Asset<T>& asset)
		{
			XYZ_ASSERT(m_Assets.find(asset.Filepath) == m_Assets.end(), "Inserting twice asset with same key");
			m_Assets[asset.Filepath] = asset;
		}

		void Remove(const std::string& key)
		{
			auto it = m_Assets.find(key);
			XYZ_ASSERT(it != m_Assets.end(), "Removing non existing asset");
			m_Assets.erase(it);
		}

		Ref<T> GetAsset(const std::string& key)
		{
			auto it = m_Assets.find(key);
			if (it == m_Assets.end())
				return Ref<T>();
			return it->second.Handle;
		}

		bool Contains(const std::string& filepath) const { return m_Assets.find(filepath) != m_Assets.end(); }

	protected:
		virtual size_t GetSize() const override { return m_Assets.size(); };


	private:
		std::unordered_map<std::string, Asset<T>> m_Assets;
	};

	class AssetManager
	{
	public:
		~AssetManager();
		
		void Serialize();

		template <typename T>
		void LoadAsset(const std::string& filepath)
		{
			size_t id = typeid(T).hash_code();
			auto storage = GetStorage<T>(id);
			if (!storage->Contains(filepath))
			{
				Asset<T> asset = { filepath };
				asset.Deserialize(*this);
				storage->Insert(asset);
			}
		}

		template <typename T>
		void RegisterAsset(const std::string& filepath, const Ref<T>& handle)
		{
			size_t id = typeid(T).hash_code();
			auto storage = GetStorage<T>(id);
			if (!storage->Contains(filepath))
			{
				Asset<T> asset = { filepath,handle };
				asset.Serialize();
				storage->Insert(asset);
			}
		}

		template <typename T>
		Ref<T> GetAsset(const std::string& filepath)
		{
			size_t id = typeid(T).hash_code();
			auto storage = GetStorage<T>(id);

			auto handle = storage->GetAsset(filepath);
			if (handle)
				return handle;
			else
			{
				Asset<T> asset = { filepath };
				asset.Deserialize(*this);
				storage->Insert(asset);
				return asset.Handle;
			}
		}

	private:
		
		template <typename T>
		AssetList<T>* GetStorage(size_t id)
		{
			if (m_AssetStorages.find(id) == m_AssetStorages.end())
			{
				AssetList<T>* list = new AssetList<T>();
				m_AssetStorages[id] = list;
				return list;
			}
			return (AssetList<T>*)(m_AssetStorages[id]);
		}

	private:
		std::unordered_map<size_t, IAssetList*> m_AssetStorages;
	};
}