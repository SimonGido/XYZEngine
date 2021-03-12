#pragma once
#include "Types.h"
#include "Component.h"

namespace XYZ {
	class ComponentAllocator
	{
	public:
		ComponentAllocator();

		void Init(uint8_t id, size_t elementSize);

		template <typename T>
		void Clone(ComponentAllocator& allocator) const
		{
			XYZ_ASSERT(IComponent::GetComponentID<T>() == m_ID && m_Initialized, "");
			allocator.Clean<T>();

			allocator.m_ID = m_ID;
			allocator.m_Size = m_Size;
			allocator.m_Capacity = m_Capacity;
			allocator.m_Buffer = new uint8_t[m_Capacity];
			for (size_t i = 0; i < m_Size; i += sizeof(T))
				new((void*)&allocator.m_Buffer[i])T(*reinterpret_cast<T*>(&m_Buffer[i]));
		}

		template <typename T>
		void Clean()
		{
			XYZ_ASSERT(IComponent::GetComponentID<T>() == m_ID && m_Initialized, "");
			for (size_t i = 0; i < m_Size; i += sizeof(T))
			{
				T* casted = reinterpret_cast<T*>(&m_Buffer[i]);
				casted->~T();
			}
			delete[]m_Buffer;
			m_Buffer = nullptr;
			m_Size = 0;
			m_Capacity = 0;
		}

		void Clean()
		{
			for (size_t i = 0; i < m_Size; i += m_ElementSize)
			{
				IComponent* casted = reinterpret_cast<IComponent*>(&m_Buffer[i]);
				casted->~IComponent();
			}
			delete[]m_Buffer;
			m_Buffer = nullptr;
			m_Size = 0;
			m_Capacity = 0;
		}

		template <typename T>
		T* Push(const T& elem)
		{
			XYZ_ASSERT(IComponent::GetComponentID<T>() == m_ID && m_Initialized, "");
			if (m_Size + sizeof(T) >= m_Capacity)
				reallocate<T>((m_Capacity * sc_CapacityMultiplier) + sizeof(T));

			T* result = new ((void*)&m_Buffer[m_Size])T(elem);
			m_Size += sizeof(T);

			return result;
		}

		template <typename T, typename ...Args>
		T* Emplace(Args&&... args)
		{
			XYZ_ASSERT(IComponent::GetComponentID<T>() == m_ID && m_Initialized, "");
			if (m_Size + sizeof(T) >= m_Capacity)
				reallocate<T>((m_Capacity * sc_CapacityMultiplier) + sizeof(T));

			T* result = new ((void*)&m_Buffer[m_Size])T(std::forward<Args>(args)...);
			m_Size += sizeof(T);

			return result;
		}

		template <typename T>
		void Erase(size_t index)
		{
			XYZ_ASSERT(IComponent::GetComponentID<T>() == m_ID && m_Initialized, "");
			size_t size = sizeof(T);

			T* removed = reinterpret_cast<T*>(&m_Buffer[index * size]);
			removed->~T();
			for (size_t i = (index + 1) * size; i < m_Size; i += size)
			{
				T* casted = reinterpret_cast<T*>(&m_Buffer[i]);
				new ((void*)&m_Buffer[i - size]) T(*casted);
				casted->~T();
			}
			m_Size -= sizeof(T);
		}

		void Erase(size_t index)
		{
			IComponent* removed = reinterpret_cast<IComponent*>(&m_Buffer[index * m_ElementSize]);

			for (size_t i = (index + 1) * m_ElementSize; i < m_Size; i += m_ElementSize)
			{
				IComponent* casted = reinterpret_cast<IComponent*>(&m_Buffer[i]);
				IComponent* dest = reinterpret_cast<IComponent*>(&m_Buffer[i - m_ElementSize]);
				dest->Copy(casted);
			}
			m_Size -= m_ElementSize;
		}

		template <typename T>
		void Pop()
		{
			Erase<T>(Size() - 1);
		}

		template <typename T>
		void Move(size_t targetIndex, size_t sourceIndex)
		{
			XYZ_ASSERT(IComponent::GetComponentID<T>() == m_ID, "");

			size_t firstIndexInBuffer = targetIndex * sizeof(T);
			size_t secondIndexInBuffer = sourceIndex * sizeof(T);
			T* first = reinterpret_cast<T*>(&m_Buffer[firstIndexInBuffer]);
			T* second = reinterpret_cast<T*>(&m_Buffer[secondIndexInBuffer]);
			new (&m_Buffer[firstIndexInBuffer])T(std::move(*second));
		}

		void Copy(size_t targetIndex, size_t sourceIndex, size_t elementSize)
		{
			IComponent* target = reinterpret_cast<IComponent*>(&m_Buffer[targetIndex * elementSize]);
			IComponent* source = reinterpret_cast<IComponent*>(&m_Buffer[sourceIndex * elementSize]);
			target->Copy(source);
		}

		template <typename T>
		T& Get(size_t index)
		{
			XYZ_ASSERT(IComponent::GetComponentID<T>() == m_ID && m_Initialized, "");
			T* buffer = (T*)m_Buffer;
			return *reinterpret_cast<T*>(buffer + index);
		}

		template <typename T>
		const T& Get(size_t index) const
		{
			XYZ_ASSERT(IComponent::GetComponentID<T>() == m_ID && m_Initialized, "");
			T* buffer = (T*)m_Buffer;
			return *reinterpret_cast<T*>(buffer + index);
		}
		bool IsInitialized() const { return m_Initialized; }

		uint8_t GetID() const { return m_ID; }
		size_t Size() const { return m_Size / m_ElementSize; }
		size_t GetElementSize() const { return m_ElementSize; }
	private:
		template <typename T>
		void reallocate(size_t newCapacity)
		{
			m_Capacity = newCapacity;
			uint8_t* tmp = new uint8_t[m_Capacity];
			for (size_t i = 0; i < m_Size; i += sizeof(T))
			{
				T* casted = reinterpret_cast<T*>(&m_Buffer[i]);
				new ((void*)&tmp[i])T(*casted);
				casted->~T();
			}
			delete[]m_Buffer;
			m_Buffer = tmp;
		}


	private:
		uint8_t* m_Buffer = nullptr;
		size_t m_Capacity = 0;
		size_t m_Size = 0;

		uint8_t m_ID;
		size_t m_ElementSize;
		bool m_Initialized = false;

		static constexpr size_t sc_CapacityMultiplier = 2;
	};
}