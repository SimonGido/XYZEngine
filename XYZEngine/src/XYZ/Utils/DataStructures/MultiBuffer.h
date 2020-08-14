#pragma once


namespace XYZ {

	template<typename... Types>
	constexpr auto GetTypesSize() 
	{
		return std::array<std::size_t, sizeof...(Types)>{sizeof(Types)...};
	}
	
	template <typename ...Types>
	class MultiBuffer
	{
	public:
		MultiBuffer(size_t capacity = 2)
			:
			m_Size(0),
			m_Capacity(capacity)
		{
			XYZ_ASSERT(capacity > 0, "Capacity of multi buffer cannot be zero");
			for (auto it : GetTypesSize<Types...>())
				m_MultiElementSize += it;
			
			
			m_Buffer = new unsigned char[m_Capacity * m_MultiElementSize];
		}


		~MultiBuffer()
		{
			delete[]m_Buffer;
		}

		void PushMultiElement(const std::tuple<Types...>& element)
		{
			std::tuple<Types...>* tmp = reinterpret_cast<std::tuple<Types...>*>(&m_Buffer[m_Size * m_MultiElementSize]);
			*tmp = element;

			m_Size++;
			if (m_Size == m_Capacity)
			{
				m_Capacity *= 2;
				reallocate();
			}
		}

		template <typename ...Args>
		void EmplaceMultiElement(Args&& ...args)
		{
			std::tuple<Types...>* tmp = reinterpret_cast<std::tuple<Types...>*>(&m_Buffer[m_Size * m_MultiElementSize]);
			*tmp = std::move(std::tuple<Args...>{ std::forward<Args>(args)... });

			m_Size++;
			if (m_Size == m_Capacity)
			{
				m_Capacity *= 2;
				reallocate();
			}
		}


		template <typename T, typename ...Args>
		void EmplaceElement(size_t index, Args&& ...args)
		{
			XYZ_ASSERT(index < m_Size, "Index out of range");
			std::tuple<Types...>* tmp = reinterpret_cast<std::tuple<Types...>*>(&m_Buffer[index * m_MultiElementSize]);
			std::get<T>(*tmp) = T(std::forward<Args>(args)...);;
		}

		void PopBack()
		{
			m_Size--;
		}

		auto operator [] (size_t index)
		{
			XYZ_ASSERT(index < m_Size, "Index out of range");
			return *reinterpret_cast<std::tuple<Types...>*>(&m_Buffer[index * m_MultiElementSize]);
		}

		const auto operator [] (size_t index) const
		{
			XYZ_ASSERT(index < m_Size, "Index out of range");
			return *reinterpret_cast<std::tuple<Types...>*>(&m_Buffer[index * m_MultiElementSize]);
		}


		template <typename T>
		T& Get(size_t index)
		{
			XYZ_ASSERT(index < m_Size, "Index out of range");
			std::tuple<Types...>* tmp = reinterpret_cast<std::tuple<Types...>*>(&m_Buffer[m_Size * m_MultiElementSize]);
			return std::get<T>(*tmp);
		}

		template <typename T>
		const T& Get(size_t index) const
		{
			XYZ_ASSERT(index < m_Size, "Index out of range");
			std::tuple<Args...>* tmp = reinterpret_cast<std::tuple<Args...>*>(&m_Buffer[m_Size * m_MultiElementSize]);
			return std::get<T>(*tmp);
		}

	private:
		
		void reallocate()
		{
			unsigned char* tmp = new unsigned char[m_Capacity * m_MultiElementSize];
			memcpy(tmp, m_Buffer, m_Size * m_MultiElementSize);
			delete[]m_Buffer;
			m_Buffer = tmp;
		}


		unsigned char* m_Buffer = nullptr;

		size_t m_Size;
		size_t m_Capacity;

		size_t m_MultiElementSize = 0;
	};
}