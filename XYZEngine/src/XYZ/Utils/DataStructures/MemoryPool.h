#pragma once
#include "XYZ/Utils/DataStructures/FreeList.h"

namespace XYZ {

	class MemoryPool
	{
	public:
		MemoryPool(const uint32_t blockSize);
		MemoryPool(const MemoryPool& other) = delete;
		MemoryPool(MemoryPool&& other) noexcept;
		~MemoryPool();

		MemoryPool& operator=(const MemoryPool& other) = delete;
		MemoryPool& operator=(MemoryPool&& other) noexcept;

		void* Allocate(uint32_t size, const char* debugName = "");
		void  Deallocate(const void* val);

	private:
		struct Block
		{
			uint8_t* Data = nullptr;
			uint32_t NextAvailableIndex = 0;
		};
		struct Chunk
		{
			Chunk() = default;
			Chunk(uint32_t size, uint32_t chunkIndex, uint8_t blockIndex);

			uint32_t Size{};	   // Size together with user data and metadata
			uint32_t ChunkIndex{}; // Index in block memory ( starts at the position of metadata )
			uint8_t  BlockIndex{}; // Index of block
		};

		struct Metadata
		{
			#ifdef XYZ_DEBUG
			uint32_t AllocIndex{};
			#endif
			uint32_t Size{}; // Size requested by user
			uint32_t ChunkIndex{};
			uint8_t  BlockIndex{};

			static constexpr uint32_t SizeTight();
		};


		void   cleanUp();
		void   sortFreeChunks();
		void   mergeFreeChunks();
		Block* createBlock();
		Chunk  findChunk(const uint32_t sizeReq);
		void   findNewBlockInUse(const uint32_t sizeReq);

		static Metadata readMetadata(const void* data);
		static void	    writeMetadata(void* data, const Metadata& metadata);

	private:
		std::vector<Block> m_Blocks;
		std::vector<Chunk> m_FreeChunks;
		FreeList<const char*> m_Allocations;

		uint8_t			   m_BlockInUse;
		uint32_t		   m_BlockSize;
		uint32_t		   m_ElementCounter;
		uint32_t		   m_MemoryUsed;

		bool m_Dirty = false;
		static constexpr size_t sc_MaxNumberOfBlocks = 255;

		friend class MemoryPoolDebug;
	};

}