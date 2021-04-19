#pragma once
#include "BasicUITypes.h"
#include "XYZ/Utils/DataStructures/Tree.h"

namespace XYZ {

	class bUIAllocator
	{
	public:
		bUIAllocator(size_t capacity);
		bUIAllocator(const bUIAllocator& other) = delete;
		bUIAllocator(bUIAllocator&& other) noexcept;
		~bUIAllocator();

		bUIAllocator& operator =(const bUIAllocator& other) = delete;
		bUIAllocator& operator =(bUIAllocator&& other) noexcept;

		void Reset();
		void Reserve(size_t size);

		template <typename T, typename ...Args>
		T* CreateElement(bUIElement* parent, Args&& ...args)
		{
			static_assert(std::is_base_of<bUIElement, T>::value, "Type T must inherit from bUIElement");
			new (&m_Data[m_Size])T(std::forward<Args>(args)...);
			T* element = reinterpret_cast<T*>(&m_Data[m_Size]);
			m_Elements.push_back({ element->Type, m_Size });
			m_ElementMap[element->Name] = m_Elements.size() - 1;
			m_Size += sizeof(T);
			if (parent)
				static_cast<bUIElement*>(element)->ID = m_Tree.Insert(element, parent->ID);
			else
				static_cast<bUIElement*>(element)->ID = m_Tree.Insert(element);

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

		template <typename T>
		T* GetElement(const std::string& name)
		{
			size_t index = m_ElementMap[name];
			return reinterpret_cast<T*>(&m_Data[m_Elements[index].Offset]);
		}

		template <typename T>
		const T* GetElement(const std::string& name) const
		{
			size_t index = m_ElementMap[name];
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
		std::unordered_map<std::string, size_t> m_ElementMap;

		size_t	 m_Size;
		size_t	 m_Capacity;
		uint8_t* m_Data;
		Tree	 m_Tree;
		static constexpr size_t sc_CapacityMultiplier = 2;


		friend class bUIData;
		friend class bUI;
	};
}