#pragma once
#include "BasicUITypes.h"

namespace XYZ {

	class bUIData
	{
	public:
		bUIData();
		bUIData(size_t capacity);
		~bUIData();

		void Reset();
		void Reserve(size_t size);

		template <typename T, typename ...Args>
		T* CreateElement(Args&& ...args)
		{
			static_assert(std::is_base_of<bUIElement, T>::value, "Type T must inherit from bUIElement");
			if (m_Size + sizeof(T) > m_Capacity)
				resize(sizeof(T));
			new (&m_Data[m_Size])T(std::forward<Args>(args)...);
			T* element = reinterpret_cast<T*>(&m_Data[m_Size]);
			m_Elements.push_back({ element->Type, m_Size });
			m_Size += sizeof(T);
			return element;
		}

		template <typename T>
		T* GetElement(size_t index)
		{
			return reinterpret_cast<T*>(&m_Data[m_Elements[index].Offset]);
		}

		template <typename T>
		const T* GetElement(size_t index) const
		{
			return reinterpret_cast<T*>(&m_Data[m_Elements[index].Offset]);
		}

		size_t Size() const
		{
			return m_Elements.size();
		}
	private:
		void resize(size_t minimalSize);
		void destroy(size_t offset);	
		void copy(bUIElementType type, size_t offset, uint8_t* buffer);

	private:
		struct Element
		{
			bUIElementType Type;
			size_t		   Offset;
		};
		std::vector<Element> m_Elements;

		size_t m_Size;
		size_t m_Capacity;
		uint8_t* m_Data;

		static constexpr size_t sc_CapacityMultiplier = 2;
	};
}