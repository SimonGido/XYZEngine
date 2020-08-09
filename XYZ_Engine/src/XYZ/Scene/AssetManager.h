#pragma once
#include "XYZ/Core/Ref.h"
#include "Serializable.h"

namespace XYZ {
	

	template <typename T>
	struct Asset
	{
		friend class AssetManager;

		Ref<T> Handle;
		
		void Serialize(AssetManager& manager);

		void Deserialize(const std::string& filepath,AssetManager& manager);
	};	
	class IAssetList
	{
	public:
		virtual ~IAssetList() {};

		virtual void Serialize(AssetManager& manager) = 0;

	protected:
		virtual size_t GetSize() const = 0;
	};


	class AssetManager;
	template <typename T>
	class AssetList : public IAssetList
	{
		friend class AssetManager;
	public:

		virtual void Serialize(AssetManager& manager) override
		{
			for (auto& asset : m_Assets)
				asset.second.Serialize(manager);		
		}

		void Insert(const Asset<T>& asset)
		{
			XYZ_ASSERT(m_Assets.find(asset.Handle->GetFilepath()) == m_Assets.end(), "Inserting twice asset with same key");
			m_Assets[asset.Handle->GetFilepath()] = asset;
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
			static_assert(std::is_base_of<Serializable, T>::value, "Trying to load asset that is not serializable");
			size_t id = typeid(T).hash_code();
			auto storage = GetStorage<T>(id);
			if (!storage->Contains(filepath))
			{
				Asset<T> asset;
				asset.Deserialize(filepath,*this);
				asset.Handle->SetFilepath(filepath);
				storage->Insert(asset);
			}
		}

		template <typename T>
		void RegisterAsset(const std::string& filepath, const Ref<T>& handle)
		{
			static_assert(std::is_base_of<Serializable, T>::value, "Trying to register asset that is not serializable");
			size_t id = typeid(T).hash_code();
			auto storage = GetStorage<T>(id);
			if (!storage->Contains(filepath))
			{
				Asset<T> asset = { handle };
				asset.Handle->SetFilepath(filepath);
				asset.Serialize(*this);
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
			{
				return handle;
			}
			else
			{
				static_assert(std::is_base_of<Serializable, T>::value, "Trying to get asset that is not serializable");
				Asset<T> asset;
				asset.Deserialize(filepath,*this);
				asset.Handle->SetFilepath(filepath);
				storage->Insert(asset);
				return asset.Handle;
			}
		}

		template <typename T>
		bool IsRegistered(const std::string& filepath) const
		{
			size_t id = typeid(T).hash_code();
			auto it = m_AssetStorages.find(id);
			if (it == m_AssetStorages.end())
				return false;
			
			AssetList<T>* list = (AssetList<T>*)(it->second);
			return list->Contains(filepath);
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