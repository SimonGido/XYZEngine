#pragma once
#include "BasicUITypes.h"
#include "BasicUIAdvancedTypes.h"
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
			size_t requiredSize = sizeof(T);
			if (m_Size + requiredSize > m_Capacity)
				resize(requiredSize);
			new (&m_Data[m_Size])T(std::forward<Args>(args)...);
			T* element = &get<T>(m_Size);
			storeElement(parent, element);
			m_Size += requiredSize;
			return element;
		}

		template <typename T, typename ...Args>
		T* CreateElement(const std::string& parentName, Args&& ...args)
		{
			static_assert(std::is_base_of<bUIElement, T>::value, "Type T must inherit from bUIElement");
			size_t requiredSize = sizeof(T);
			if (m_Size + requiredSize > m_Capacity)
				resize(requiredSize);
			new (&m_Data[m_Size])T(std::forward<Args>(args)...);
			T* element = &get<T>(m_Size);
			bUIElement* parent = GetElement<bUIElement>(parentName);
			storeElement(parent, element);
			m_Size += requiredSize;
			return element;
		}

		template <typename T>
		T* GetElement(size_t index)
		{
			return &get<T>(m_Elements[index].Offset);
		}

		template <typename T>
		const T* GetElement(size_t index) const
		{
			return &get<T>(m_Elements[index].Offset);
		}

		template <typename T>
		T* GetElement(const std::string& name)
		{
			size_t index = m_ElementMap[name];
			return &get<T>(m_Elements[index].Offset);
		}

		template <typename T>
		const T* GetElement(const std::string& name) const
		{
			size_t index = m_ElementMap[name];
			return &get<T>(m_Elements[index].Offset);
		}

		size_t Size() const
		{
			return m_Elements.size();
		}

		Tree& GetHierarchy() { return m_Tree; }
	private:
		void resize(size_t minimalSize);
		void destroy(size_t offset);	
		void moveElement(uint8_t* dest, bUIElementType type, size_t offset);	
		void storeElement(bUIElement* parent, bUIElement* element);

		template <typename T>
		void move(uint8_t* dest, size_t offset)
		{
			new(&dest[offset])T(std::move(get<T>(offset)));
		}
		template <typename T>
		void copy(uint8_t* dest, size_t offset)
		{
			new(&dest[offset])T(get<T>(offset));
		}
		template <typename T>
		T& get(size_t offset)
		{
			return *reinterpret_cast<T*>(&m_Data[offset]);
		}
		template <typename T>
		const T& get(size_t offset) const
		{
			return *reinterpret_cast<T*>(&m_Data[offset]);
		}
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
		uint8_t* m_Data = nullptr;
		Tree	 m_Tree;
		static constexpr size_t sc_CapacityMultiplier = 2;


		friend class bUIData;
		friend class bUI;
	};
}