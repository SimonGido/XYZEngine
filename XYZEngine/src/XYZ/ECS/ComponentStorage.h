#pragma once
#include "Types.h"
#include "Entity.h"
#include "Serialization/ByteStream.h"

namespace XYZ {

	class IComponentStorage
	{
	public:
		IComponentStorage() = default;
		IComponentStorage(const IComponentStorage& other);
		IComponentStorage(IComponentStorage&& other) noexcept;

		virtual					  ~IComponentStorage() = default;
		virtual void			   Clear() = 0;
		virtual IComponentStorage* Move(uint8_t* buffer) = 0;
		virtual IComponentStorage* Move() = 0;
		virtual IComponentStorage* Copy(uint8_t* buffer) const = 0;
		virtual IComponentStorage* Copy() const = 0;

		virtual void			   AddRawComponent(Entity entity, const ByteStream& in) = 0;
		virtual void			   CopyComponentData(Entity entity, ByteStream& out) const = 0;
		virtual void			   UpdateComponentData(Entity entity, const ByteStream& in) = 0;
		
		virtual Entity			   EntityDestroyed(Entity entity) = 0;
		virtual uint32_t		   GetComponentIndex(Entity entity) const = 0;
		virtual Entity			   GetEntityAtIndex(size_t index) const = 0;

		virtual size_t			   Size() const = 0;
		virtual uint16_t		   ID() const = 0;

		virtual const std::vector<Entity>& GetDataEntityMap() const = 0;

		template <typename FuncT, typename ...Args>
		void	 AddOnConstruction(FuncT&& func, Args&& ...args);

		template <typename Type>
		void	 AddOnConstruction(void(Type::* func)(), Type* instance);

		template <typename FuncT>
		void	 RemoveOnConstruction(FuncT&& func);

		template <typename Type>
		void	 RemoveOnConstruction(void(Type::* func)(), Type* instance);

		template <typename Type>
		void	 RemoveOnConstructionOfInstance(Type* instance);


		template <typename FuncT, typename ...Args>
		void	 AddOnDestruction(FuncT&& func, Args&& ...args);

		template <typename Type>
		void	 AddOnDestruction(void(Type::* func)(), Type* instance);

		template <typename FuncT>
		void	 RemoveOnDestruction(FuncT&& func);

		template <typename Type>
		void	 RemoveOnDestruction(void(Type::* func)(), Type* instance);

		template <typename Type>
		void	 RemoveOnDestructionOfInstance(Type* instance);

	protected:
		struct Callback
		{
			Callback() = default;
			Callback(const std::function<void()>& callable, void* funcPtr, void* instance);

			std::function<void()> Callable;
			void* FunctionPointer = nullptr;
			void* Instance = nullptr;
		};

		std::vector<Callback> m_OnConstruction;
		std::vector<Callback> m_OnDestruction;
	};

	template <typename T>
	class ComponentStorage : public IComponentStorage
	{
	public:
		ComponentStorage() = default;
		ComponentStorage(const ComponentStorage<T>& other);		
		ComponentStorage(ComponentStorage<T>&& other) noexcept;
			

		virtual void			   Clear() override;	
		virtual IComponentStorage* Move(uint8_t* buffer) override;
		virtual IComponentStorage* Move() override;
		virtual IComponentStorage* Copy(uint8_t* buffer) const override;		
		virtual IComponentStorage* Copy() const override;
		
		virtual void	 AddRawComponent(Entity entity, const ByteStream& in) override;	
		virtual void	 CopyComponentData(Entity entity, ByteStream& out) const override;	
		virtual void	 UpdateComponentData(Entity entity, const ByteStream& in) override;
		
		virtual Entity   EntityDestroyed(Entity entity) override;	
		virtual uint32_t GetComponentIndex(Entity entity) const override;	
		virtual Entity   GetEntityAtIndex(size_t index) const override;
		
		virtual size_t   Size() const override;	
		virtual uint16_t ID() const override;
		virtual const std::vector<Entity>& GetDataEntityMap() const override;
	
		template <typename ...Args>
		T&		 EmplaceComponent(Entity entity, Args&& ... args);	
		T&		 AddComponent(Entity entity, const T& component);	
		uint32_t RemoveComponent(Entity entity);

	
		T&		 GetComponent(Entity entity);	
		const T& GetComponent(Entity entity) const;	
		T&		 GetComponentAtIndex(size_t index);
		const T& GetComponentAtIndex(size_t index) const;
		
		T&		 operator[](size_t index);	
		const T& operator[](size_t index) const;
		

		typename std::vector<T>::iterator begin() { return m_Data.begin(); }
		typename std::vector<T>::iterator end() { return m_Data.end(); }
		typename std::vector<T>::const_iterator begin() const { return m_Data.begin(); }
		typename std::vector<T>::const_iterator end()   const { return m_Data.end(); }

	private:
		std::vector<T>		  m_Data;
		std::vector<Entity>   m_DataEntityMap;
		std::vector<uint32_t> m_EntityDataMap;

	
		friend class ECSSerializer;
	};

	template<typename FuncT, typename ...Args>
	inline void IComponentStorage::AddOnConstruction(FuncT&& func, Args && ...args)
	{
		m_OnConstruction.emplace_back([=]() {
			func((args)...);
		}, func, nullptr);
	}


	template<typename Type>
	inline void IComponentStorage::AddOnConstruction(void(Type::* func)(), Type* instance)
	{
		m_OnConstruction.emplace_back([=]() {
			(instance->*func)();
		}, &func, instance);
	}

	template<typename FuncT>
	inline void IComponentStorage::RemoveOnConstruction(FuncT&& func)
	{
		for (size_t i = 0; i < m_OnConstruction.size(); ++i)
		{
			if (m_OnConstruction[i].FunctionPointer == func)
			{
				m_OnConstruction.erase(m_OnConstruction.begin() + i);
				return;
			}
		}
	}

	template<typename Type>
	inline void IComponentStorage::RemoveOnConstruction(void(Type::* func)(), Type* instance)
	{
		for (size_t i = 0; i < m_OnConstruction.size(); ++i)
		{
			if (m_OnConstruction[i].FunctionPointer == &func && m_OnConstruction[i].Instance == instance)
			{
				m_OnConstruction.erase(m_OnConstruction.begin() + i);
				return;
			}
		}
	}

	template<typename Type>
	inline void IComponentStorage::RemoveOnConstructionOfInstance(Type* instance)
	{
		for (auto it = m_OnConstruction.begin(); it != m_OnConstruction.end();)
		{
			if (it->Instance == instance)
			{
				it = m_OnCon
					struction.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	template<typename FuncT, typename ...Args>
	inline void IComponentStorage::AddOnDestruction(FuncT&& func, Args && ...args)
	{
		m_OnDestruction.emplace_back([=]() {
			func((args)...);
		}, func, nullptr);
	}

	template<typename Type>
	inline void IComponentStorage::AddOnDestruction(void(Type::* func)(), Type* instance)
	{
		m_OnDestruction.emplace_back([=]() {
			(instance->*func)();
		}, &func, instance);
	}


	template<typename FuncT>
	inline void IComponentStorage::RemoveOnDestruction(FuncT&& func)
	{
		for (size_t i = 0; i < m_OnDestruction.size(); ++i)
		{
			if (m_OnDestruction[i].FunctionPointer == func)
			{
				m_OnDestruction.erase(m_OnDestruction.begin() + i);
				return;
			}
		}
	}

	template<typename Type>
	inline void IComponentStorage::RemoveOnDestruction(void(Type::* func)(), Type* instance)
	{
		for (size_t i = 0; i < m_OnDestruction.size(); ++i)
		{
			if (m_OnDestruction[i].FunctionPointer == &func && m_OnDestruction[i].Instance == instance)
			{
				m_OnDestruction.erase(m_OnDestruction.begin() + i);
				return;
			}
		}
	}

	template<typename Type>
	inline void IComponentStorage::RemoveOnDestructionOfInstance(Type* instance)
	{
		for (auto it = m_OnDestruction.begin(); it != m_OnDestruction.end();)
		{
			if (it->Instance == instance)
			{
				it = m_OnDestruction.erase(it);
			}
			else
			{
				it++;
			}
		}
	}


	inline IComponentStorage::Callback::Callback(const std::function<void()>& callable, void* funcPtr, void* instance)
		:
		Callable(callable),
		FunctionPointer(funcPtr),
		Instance(instance)
	{
	}



	template<typename T>
	inline ComponentStorage<T>::ComponentStorage(const ComponentStorage<T>& other)
		:
		IComponentStorage(other),
		m_Data(other.m_Data),
		m_DataEntityMap(other.m_DataEntityMap),
		m_EntityDataMap(other.m_EntityDataMap)
	{}
	template<typename T>
	inline ComponentStorage<T>::ComponentStorage(ComponentStorage<T> && other) noexcept
		:
		IComponentStorage(std::move(other)),
		m_Data(std::move(other.m_Data)),
		m_DataEntityMap(std::move(other.m_DataEntityMap)),
		m_EntityDataMap(std::move(other.m_EntityDataMap))
	{}
	template<typename T>
	inline void ComponentStorage<T>::Clear()
	{
		m_DataEntityMap.clear();
		m_EntityDataMap.clear();
		m_Data.clear();
		m_OnConstruction.clear();
		m_OnDestruction.clear();
	}
	template<typename T>
	inline IComponentStorage* ComponentStorage<T>::Move(uint8_t* buffer)
	{
		return new (buffer)ComponentStorage<T>(std::move(*this));
	}
	template<typename T>
	inline IComponentStorage* ComponentStorage<T>::Move()
	{
		return new ComponentStorage<T>(std::move(*this));
	}
	template<typename T>
	inline IComponentStorage* ComponentStorage<T>::Copy(uint8_t* buffer) const
	{
		return new ((void*)buffer)ComponentStorage<T>(*this);
	}
	template<typename T>
	inline IComponentStorage* ComponentStorage<T>::Copy() const
	{
		return new ComponentStorage<T>(*this);
	}
	template<typename T>
	inline void ComponentStorage<T>::AddRawComponent(Entity entity, const ByteStream& in)
	{
		T component;
		in >> component;
		AddComponent(entity, component);
	}
	template<typename T>
	inline void ComponentStorage<T>::CopyComponentData(Entity entity, ByteStream& out) const
	{
		out << m_Data[m_EntityDataMap[(size_t)entity]];
	}
	template<typename T>
	inline void ComponentStorage<T>::UpdateComponentData(Entity entity, const ByteStream& in)
	{
		in >> m_Data[m_EntityDataMap[(size_t)entity]];
	}
	template<typename T>
	inline Entity ComponentStorage<T>::EntityDestroyed(Entity entity)
	{
		return RemoveComponent(entity);
	}
	template<typename T>
	inline uint32_t ComponentStorage<T>::GetComponentIndex(Entity entity) const
	{
		return m_EntityDataMap[(size_t)entity];
	}
	template<typename T>
	inline Entity ComponentStorage<T>::GetEntityAtIndex(size_t index) const
	{
		return m_DataEntityMap[index];
	}
	template<typename T>
	inline size_t ComponentStorage<T>::Size() const
	{
		return m_Data.size();
	}
	template<typename T>
	inline uint16_t ComponentStorage<T>::ID() const
	{
		return Component<T>::ID();
	}

	template<typename T>
	inline const std::vector<Entity>& ComponentStorage<T>::GetDataEntityMap() const
	{
		return m_DataEntityMap;
	}

	template<typename T>
	template<typename ...Args>
	inline T& ComponentStorage<T>::EmplaceComponent(Entity entity, Args && ...args)
	{
		if (m_EntityDataMap.size() <= (uint32_t)entity)
			m_EntityDataMap.resize((uint32_t)entity + 1);

		m_DataEntityMap.push_back(entity);
		m_EntityDataMap[(size_t)entity] = (uint32_t)m_Data.size();
		m_Data.emplace_back(std::forward<Args>(args)...);
		return m_Data.back();
	}

	template<typename T>
	inline T& ComponentStorage<T>::AddComponent(Entity entity, const T& component)
	{
		if (m_EntityDataMap.size() <= (uint32_t)entity)
			m_EntityDataMap.resize((uint32_t)entity + 1);

		m_DataEntityMap.push_back(entity);
		m_EntityDataMap[(size_t)entity] = (uint32_t)m_Data.size();
		m_Data.push_back(component);
		for (auto& callback : m_OnConstruction)
			callback.Callable();
		return m_Data.back();
	}

	template<typename T>
	inline uint32_t ComponentStorage<T>::RemoveComponent(Entity entity)
	{
		Entity updatedEntity;
		if (entity != m_DataEntityMap.back())
		{
			// Entity of last element in data pack
			Entity lastEntity = m_DataEntityMap.back();
			// Index that is entity pointing to
			uint32_t index = m_EntityDataMap[(size_t)entity];
			// Move last element in data pack at the place of removed component
			m_Data[index] = std::move(m_Data.back());
			// Point data entity map at index to last entity
			m_DataEntityMap[index] = lastEntity;
			// Point last entity to data new index;
			m_EntityDataMap[(size_t)lastEntity] = index;
			// Pop back last element
			updatedEntity = lastEntity;
		}
		m_Data.pop_back();
		m_DataEntityMap.pop_back();
		for (auto& callback : m_OnDestruction)
			callback.Callable();

		return updatedEntity;
	}

	template<typename T>
	inline T& ComponentStorage<T>::GetComponent(Entity entity)
	{
		return m_Data[m_EntityDataMap[(size_t)entity]];
	}

	template<typename T>
	inline const T& ComponentStorage<T>::GetComponent(Entity entity) const
	{
		return m_Data[m_EntityDataMap[(size_t)entity]];
	}

	template<typename T>
	inline T& ComponentStorage<T>::GetComponentAtIndex(size_t index)
	{
		return m_Data[index];
	}

	template<typename T>
	inline const T& ComponentStorage<T>::GetComponentAtIndex(size_t index) const
	{
		return m_Data[index];
	}

	template<typename T>
	inline T& ComponentStorage<T>::operator[](size_t index)
	{
		return m_Data[index];
	}

	template<typename T>
	inline const T& ComponentStorage<T>::operator[](size_t index) const
	{
		return m_Data[index];
	}
	



}