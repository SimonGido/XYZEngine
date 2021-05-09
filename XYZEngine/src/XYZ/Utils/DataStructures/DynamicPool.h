#pragma once


namespace XYZ {

	class DynamicPool
	{
	public:
		struct Base
		{
			virtual ~Base() = default;
			virtual void OnCopy(uint8_t* buffer) = 0;
		};
	public:
		DynamicPool(uint32_t capacity = 0);	
		DynamicPool(const DynamicPool& other);
		DynamicPool(DynamicPool&& other) noexcept;
		~DynamicPool();
		
		template <typename T>
		void Push(const T& elem)
		{
			static_assert(std::is_base_of<Base, T>::value, "BaseType is not base type of type T");
			if (m_Size + sizeof(T) > m_Capacity)
				reallocate(sizeof(T));
			new(&m_Data[m_Size])T(elem);
			m_Handles.push_back(m_Size);
			m_Size += sizeof(T);
		}

		template <typename T, typename ...Args>
		void Emplace(Args&& ...args)
		{
			static_assert(std::is_base_of<Base, T>::value, "BaseType is not base type of type T");
			if (m_Size + sizeof(T) > m_Capacity)
				reallocate(sizeof(T));
			new(&m_Data[m_Size])T(std::forward<Args>(args)...);
			m_Handles.push_back(m_Size);
			m_Size += sizeof(T);
		}

		void Erase(size_t index);

		template <typename T>
		T& Get(size_t index)
		{
			static_assert(std::is_base_of<Base, T>::value, "BaseType is not base type of type T");
			return *reinterpret_cast<T*>(&m_Data[m_Handles[index]]);
		}
		template <typename T>
		const T& Get(size_t index) const
		{
			static_assert(std::is_base_of<Base, T>::value, "BaseType is not base type of type T");
			return *reinterpret_cast<T*>(&m_Data[m_Handles[index]]);
		}
		Base& Back();
		const Base& Back() const;
		size_t Size() const { return m_Handles.size(); }
		
		Base& operator[](size_t index);
		const Base& operator[](size_t index) const;
		
	private:
		void destroy();
		void reallocate(size_t minSize);
		

	private:
		size_t m_Size;
		size_t m_Capacity;
		uint8_t* m_Data;

		std::vector<size_t> m_Handles;
		static constexpr size_t sc_CapacityMultiplier = 2;
	};

}