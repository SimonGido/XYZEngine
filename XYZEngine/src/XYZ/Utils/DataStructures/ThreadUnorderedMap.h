#pragma once

#include <unordered_map>
#include <shared_mutex>


namespace XYZ {
	template <typename Key, typename Value, typename HashFunc = std::hash<Key>>
	class ThreadUnorderedMap
	{
	public:
		void Set(const Key& key, const Value& val)
		{
			std::unique_lock lock(m_Mutex);
			m_UnorderedMap[key] = val;
		}

		Value Get(const Key& key) const
		{
			std::shared_lock lock(m_Mutex);
			return m_UnorderedMap.at(key);
		}

		bool TryGet(const Key& key, Value& value) const
		{
			std::shared_lock lock(m_Mutex);
			auto it = m_UnorderedMap.find(key);
			if (it != m_UnorderedMap.end())
			{
				value = it->second;
				return true;
			}
			return false;
		}

		bool Erase(const Key& key)
		{
			std::unique_lock lock(m_Mutex);
			auto it = m_UnorderedMap.find(key);
			if (it != m_UnorderedMap.end())
			{
				m_UnorderedMap.erase(it);
				return true;
			}
			return false;
		}

		bool Contains(const Key& key) const
		{
			std::shared_lock lock(m_Mutex);
			return m_UnorderedMap.find(key) != m_UnorderedMap.end();
		}

		void Clear()
		{
			std::unique_lock lock(m_Mutex);
			m_UnorderedMap.clear();
		}

		template <typename Func>
		void ForEach(Func&& func) const
		{
			std::shared_lock lock(m_Mutex);
			for (auto& [key, val] : m_UnorderedMap)
			{
				func(key, val);
			}
		}

		bool Find(const Key& key, Value& value) const
		{
			std::shared_lock lock(m_Mutex);
			auto it = m_UnorderedMap.find(key);
			if (it != m_UnorderedMap.end())
			{
				value = it->second;
				return true;
			}
			return false;
		}

		const auto find(const Key& key) const
		{
			std::shared_lock lock(m_Mutex);
			return m_UnorderedMap.find(key);
		}

		const auto end() const
		{
			std::shared_lock lock(m_Mutex);
			return m_UnorderedMap.end();
		}


		

	private:
		mutable std::shared_mutex m_Mutex;

		std::unordered_map<Key, Value, HashFunc> m_UnorderedMap;
	};
}