#pragma once

namespace XYZ {
	
	class Pool
	{
	public:
		Pool(size_t size);
		~Pool();

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
			return reinterpret_cast<T*>((void*)&m_Data[offset]);
		}

		template <typename T>
		void Destroy(T* instance)
		{
			instance->~T();
		}

	private:
		uint8_t* m_Data;
		size_t m_Size;
	};
}