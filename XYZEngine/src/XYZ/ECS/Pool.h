#pragma once

#include <type_traits>

namespace XYZ {
	
	class Pool
	{
	public:
		Pool(size_t size);
		Pool(Pool&& other) noexcept;
		~Pool();

		Pool& operator =(Pool&& other) noexcept;

		template <typename T, typename ...Args>
		T* Allocate(size_t offset, Args&& ...args)
		{
			return new((void*)&m_Data[offset])T(std::forward<Args>(args)...);
		}

		template <typename T>
		T* Get(size_t offset)
		{
			return reinterpret_cast<T*>((void*)&m_Data[offset]);
		}

		template <typename T>
		const T* Get(size_t offset) const
		{
			return reinterpret_cast<const T*>((void*)&m_Data[offset]);
		}

		template <typename T>
		void Destroy(T* instance)
		{
			instance->~T();
		}
	
		operator uint8_t*()
		{
			return m_Data;
		}
		operator const uint8_t*() const
		{
			return m_Data;
		}
	private:
		uint8_t* m_Data;
		size_t m_Size;

		static constexpr size_t sc_CapacityMultiplier = 2;
	};
}