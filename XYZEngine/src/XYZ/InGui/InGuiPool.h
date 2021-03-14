#pragma once


namespace XYZ {

	class InGuiPool
	{
	private:
		struct Chunk
		{
			Chunk(uint8_t* data)
				: Data(data)
			{}
			uint8_t* Data;
			size_t   Size = 0;

			template <typename T, typename ...Args>
			T* Allocate(Args&&...args)
			{
				T* tmp = new ((void*)&Data[Size])T(std::forward<Args>(args)...);
				Size += sizeof(T);
				return tmp;
			}
			template <typename T>
			T* Pop()
			{
				T* tmp = reinterpret_cast<T*>(&Data[Size - sizeof(T)]);
				Size -= sizeof(T);
				return tmp;
			}
			template <typename T>
			T* Last()
			{
				return reinterpret_cast<T*>(&Data[Size - sizeof(T)]);
			}
		};
	
	public:
		InGuiPool(size_t chunkCapacity);
		~InGuiPool();

		template <typename T, typename ...Args>
		T* Allocate(Args&& ...args)
		{
			size_t size = sizeof(T);
			XYZ_ASSERT(size < m_Capacity, "Attempting to allocate object bigger than chunk capacity");
			if (m_Chunks.empty())
				createNewChunk();
			Chunk* last = &m_Chunks.back();
			if (last->Size + size >= m_Capacity)
			{
				createNewChunk();
				last = &m_Chunks.back();
			}
			return last->Allocate<T>(std::forward<Args>(args)...);
		}

		template <typename T>
		T* Pop()
		{
			XYZ_ASSERT(m_Chunks.size(), "");
			Chunk* last = &m_Chunks.back();
			
			T* tmp = last->Pop<T>();
			if (!last->Size)
			{
				m_EmptyChunks.push_back(*last);
				m_Chunks.pop_back();
			}
			return tmp;
		}

		template <typename T>
		T* Last()
		{
			XYZ_ASSERT(m_Chunks.size(), "");
			Chunk* last = &m_Chunks.back();
			return last->Last<T>();
		}
	private:
		void createNewChunk();

	private:
		std::vector<Chunk> m_Chunks;
		std::vector<Chunk> m_EmptyChunks;

		const size_t m_Capacity;
	};
}