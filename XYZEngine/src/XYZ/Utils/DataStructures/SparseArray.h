#pragma once

#include <optional>

namespace XYZ {

	class OptionalIndex
	{
	public:
		OptionalIndex() = default;
		OptionalIndex(uint32_t index);
			
		OptionalIndex& operator=(uint32_t index);

		bool IsValid() const { return m_Value.has_value(); }

		operator uint32_t () const { return m_Value.value(); }
	private:
		std::optional<uint32_t> m_Value;
	};

	template <typename T>
	class SparseArray
	{
	public:
		SparseArray() = default;
		SparseArray(const SparseArray<T>& other);
		SparseArray(SparseArray<T>&& other) noexcept;

		SparseArray<T>& operator =(const SparseArray<T>& other);
		SparseArray<T>& operator =(SparseArray<T>&& other) noexcept;

		void Push(uint32_t entity, const T& value);

		template <typename ...Args>
		void Emplace(uint32_t entity, Args&& ...args);

		void Erase(uint32_t entity);
		void Reserve(uint32_t size);
		void Clear();

		bool IsValid(uint32_t entity) const;

		T& Back()			  { return m_Data.back(); }
		const T& Back() const { return m_Data.back(); };

		uint32_t GetDataIndex(uint32_t entity)  const { return m_EntityDataMap[entity]; }
		uint32_t GetEntityIndex(uint32_t index) const { return m_DataEntityMap[index]; }
		size_t   Size()							const { return m_Data.size(); }

		T& operator[](size_t index) { return m_Data[index]; }
		const T& operator[](size_t index) const { return m_Data[index]; }

		typename std::vector<T>::iterator begin() { return m_Data.begin(); }
		typename std::vector<T>::iterator end() { return m_Data.end(); }
		typename std::vector<T>::const_iterator begin() const { return m_Data.begin(); }
		typename std::vector<T>::const_iterator end()   const { return m_Data.end(); }
	private:
		std::vector<T>			   m_Data;
		std::vector<OptionalIndex> m_EntityDataMap;
		std::vector<OptionalIndex> m_DataEntityMap;
	};

	template<typename T>
	inline SparseArray<T>::SparseArray(const SparseArray<T>& other)
		:
		m_Data(other.m_Data),
		m_EntityDataMap(other.m_EntityDataMap),
		m_DataEntityMap(other.m_DataEntityMap)
	{
	}
	template<typename T>
	inline SparseArray<T>::SparseArray(SparseArray<T>&& other) noexcept
		:
		m_Data(std::move(other.m_Data)),
		m_EntityDataMap(std::move(other.m_EntityDataMap)),
		m_DataEntityMap(std::move(other.m_DataEntityMap))
	{
	}
	template<typename T>
	inline SparseArray<T>& SparseArray<T>::operator=(const SparseArray<T>& other)
	{
		m_Data = other.m_Data;
		m_EntityDataMap = other.m_EntityDataMap;
		m_DataEntityMap = other.m_DataEntityMap;
		return *this;
	}
	template<typename T>
	inline SparseArray<T>& SparseArray<T>::operator=(SparseArray<T>&& other) noexcept
	{
		m_Data = std::move(other.m_Data);
		m_EntityDataMap = std::move(other.m_EntityDataMap);
		m_DataEntityMap = std::move(other.m_DataEntityMap);
		return *this;
	}
	template<typename T>
	inline void SparseArray<T>::Push(uint32_t entity, const T& value)
	{
		if (m_EntityDataMap.size() <= static_cast<uint32_t>(entity))
			m_EntityDataMap.resize(static_cast<uint32_t>(entity) + 1);


		if (m_EntityDataMap[entity].IsValid())
		{
			m_Data[m_EntityDataMap[entity]] = value;
		}
		else
		{
			m_DataEntityMap.push_back(entity);
			m_EntityDataMap[entity] = m_Data.size();
			m_Data.push_back(value);
		}
	}
	template<typename T>
	inline void SparseArray<T>::Erase(uint32_t entity)
	{
		if (entity != m_DataEntityMap.back())
		{
			// Entity of last element in data pack
			const uint32_t lastEntity = m_DataEntityMap.back();
			// Index that is entity pointing to
			uint32_t index = m_EntityDataMap[entity];
			// Move last element in data pack at the place of removed component
			m_Data[index] = std::move(m_Data.back());
			// Point data entity map at index to last entity
			m_DataEntityMap[index] = lastEntity;
			// Point last entity to data new index;
			m_EntityDataMap[lastEntity] = index;
		}
		m_EntityDataMap[entity] = Index();
		m_Data.pop_back();
		m_DataEntityMap.pop_back();
	}
	template<typename T>
	inline void SparseArray<T>::Clear()
	{
		m_Data.clear();
		m_DataEntityMap.clear();
		m_EntityDataMap.clear();
	}
	template<typename T>
	inline bool SparseArray<T>::IsValid(uint32_t entity) const
	{
		if (m_EntityDataMap.size() <= entity)
			return false;
		return m_EntityDataMap[entity].IsValid();
	}
	template<typename T>
	inline void SparseArray<T>::Reserve(uint32_t size)
	{
		m_Data.reserve(size);
		m_DataEntityMap.reserve(size);
	}
	template<typename T>
	template<typename ...Args>
	inline void SparseArray<T>::Emplace(uint32_t entity, Args && ...args)
	{
		if (m_EntityDataMap.size() <= static_cast<uint32_t>(entity))
			m_EntityDataMap.resize(static_cast<uint32_t>(entity) + 1);


		if (m_EntityDataMap[entity].IsValid())
		{
			m_Data[m_EntityDataMap[entity]] = T(std::forward<Args>(args)...);
		}
		else
		{
			m_DataEntityMap.push_back(entity);
			m_EntityDataMap[entity] = m_Data.size();
			m_Data.emplace_back(std::forward<Args>(args)...);
		}
	}
}
