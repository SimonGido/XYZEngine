#include "stdafx.h"
#include "MemoryPool.h"

#include "XYZ/Debug/Profiler.h"

namespace XYZ {

	MemoryPool::Chunk::Chunk(uint32_t size, uint32_t chunkIndex, uint8_t blockIndex)
		: Size(size), ChunkIndex(chunkIndex), BlockIndex(blockIndex)
	{}


	MemoryPool::MemoryPool(const uint32_t blockSize)
		:
		m_BlockSize(blockSize),
		m_ElementCounter(0),
		m_MemoryUsed(0)
	{
		createBlock();
	}
	MemoryPool::MemoryPool(MemoryPool&& other) noexcept
		:
		m_Blocks(std::move(other.m_Blocks)),
		m_FreeChunks(std::move(other.m_FreeChunks)),
		m_BlockInUse(other.m_BlockInUse),
		m_BlockSize(other.m_BlockSize),
		m_ElementCounter(other.m_ElementCounter),
		m_MemoryUsed(other.m_MemoryUsed),
		m_Dirty(other.m_Dirty)
	{
	}
	MemoryPool::~MemoryPool()
	{
		if (m_ElementCounter == 0 || m_MemoryUsed == 0)
			XYZ_WARN("Memory not released, number of elements: {} not released memory: {}", m_ElementCounter, m_MemoryUsed);

		#ifdef XYZ_DEBUG
		for (int32_t i = 0; i < m_Allocations.Range(); ++i)
		{
			if (m_Allocations.Valid(i) && m_Allocations[i])
				XYZ_WARN("Allocation with name {} not released", m_Allocations[i]);
		}
		#endif //  XYZ_DEBUG

		for (auto& block : m_Blocks)
		{
			if (block.Data)
				delete[]block.Data;
		}
	}
	MemoryPool& MemoryPool::operator=(MemoryPool&& other) noexcept
	{
		m_Blocks = std::move(other.m_Blocks);
		m_FreeChunks = std::move(other.m_FreeChunks);
		m_BlockInUse = other.m_BlockInUse;
		m_BlockSize = other.m_BlockSize;
		m_ElementCounter = other.m_ElementCounter;
		m_MemoryUsed = other.m_MemoryUsed;
		m_Dirty = other.m_Dirty;
		return *this;
	}

	void* MemoryPool::Allocate(uint32_t size, const char* debugName)
	{
		XYZ_PROFILE_FUNC("MemoryPool::Allocate");
		const uint32_t sizeReq = size + Metadata::SizeTight();
		m_ElementCounter++;
		m_MemoryUsed += sizeReq;

		if (m_Dirty)
			cleanUp();

		Chunk chunk = findChunk(sizeReq);
		void* handle = &m_Blocks[chunk.BlockIndex].Data[chunk.ChunkIndex + Metadata::SizeTight()];
		#ifdef XYZ_DEBUG
		writeMetadata(handle, Metadata{ static_cast<uint32_t>(m_Allocations.Insert(debugName)), size, chunk.ChunkIndex, chunk.BlockIndex });
		#else
		writeMetadata(handle, Metadata{ size, chunk.ChunkIndex, chunk.BlockIndex });
		#endif
		return handle;
	}

	void MemoryPool::Deallocate(const void* val)
	{
		XYZ_PROFILE_FUNC("MemoryPool::Deallocate");
		Metadata metadata = readMetadata(val);
		m_ElementCounter--;
		m_Dirty = true;

		const uint32_t chunkSize = metadata.Size + Metadata::SizeTight();
		m_MemoryUsed -= chunkSize;
		m_FreeChunks.emplace_back(chunkSize, metadata.ChunkIndex, metadata.BlockIndex);
		
		#ifdef XYZ_DEBUG
		m_Allocations.Erase(metadata.AllocIndex);
		#endif
	}

	void MemoryPool::cleanUp()
	{
		XYZ_PROFILE_FUNC("MemoryPool::cleanUp");
		sortFreeChunks();
		mergeFreeChunks();
		m_Dirty = false;
	}

	void MemoryPool::sortFreeChunks()
	{
		XYZ_PROFILE_FUNC("MemoryPool::sortFreeChunks");
		std::sort(m_FreeChunks.begin(), m_FreeChunks.end(), [](const Chunk& a, const Chunk& b) {
			if (a.BlockIndex == b.BlockIndex)
				return a.ChunkIndex < b.ChunkIndex;
			return a.BlockIndex < b.BlockIndex;
		});
	}

	void MemoryPool::mergeFreeChunks()
	{
		XYZ_PROFILE_FUNC("MemoryPool::mergeFreeChunks");
		for (int64_t i = m_FreeChunks.size() - 1; i >= 0; --i)
		{
			Chunk& lastChunk = m_FreeChunks[i];
			Block& block = m_Blocks[lastChunk.BlockIndex];
			uint32_t lastChunkEnd = lastChunk.ChunkIndex + lastChunk.Size;

			if (lastChunkEnd == block.NextAvailableIndex) // it is last chunk in block
			{
				block.NextAvailableIndex -= lastChunk.Size;
				m_FreeChunks.erase(m_FreeChunks.begin() + i);
			}
			else if (i > 0)
			{
				Chunk& prevChunk = m_FreeChunks[i - 1];
				// last chunk and prev chunk do not share same block index
				if (lastChunk.BlockIndex != prevChunk.BlockIndex)
					continue;

				uint32_t prevChunkEnd = prevChunk.ChunkIndex + prevChunk.Size;
				// previous chunk is not connected to last chunk
				if (prevChunkEnd != lastChunk.ChunkIndex)
					continue;

				// merge chunks
				prevChunk.Size += lastChunk.Size;
				m_FreeChunks.erase(m_FreeChunks.begin() + i);
			}
		}
	}

	MemoryPool::Block* MemoryPool::createBlock()
	{
		XYZ_PROFILE_FUNC("MemoryPool::createBlock");
		m_Blocks.push_back(Block());
		m_Blocks.back().Data = new uint8_t[m_BlockSize];
		memset(m_Blocks.back().Data, 0, m_BlockSize);
		m_BlockInUse = static_cast<uint8_t>(m_Blocks.size()) - 1;
		return &m_Blocks.back();
	}

	MemoryPool::Chunk MemoryPool::findChunk(const uint32_t sizeReq)
	{
		XYZ_PROFILE_FUNC("MemoryPool::findChunk");
		// Try to find in free chunks
		for (int64_t i = m_FreeChunks.size() - 1; i >= 0; --i)
		{
			Chunk& chunk = m_FreeChunks[i];
			if (chunk.Size > sizeReq)
			{
				Chunk result(sizeReq, chunk.ChunkIndex, chunk.BlockIndex);
				chunk.Size -= sizeReq;
				return result;
			}
			else if (chunk.Size == sizeReq)
			{
				Chunk result(chunk);
				m_FreeChunks.erase(m_FreeChunks.begin() + i);
				return result;
			}
		}

		Block* inUse = &m_Blocks[m_BlockInUse];
		if (inUse->NextAvailableIndex + sizeReq > m_BlockSize)
		{
			findNewBlockInUse(sizeReq);
			inUse = &m_Blocks[m_BlockInUse];
		}
		Chunk result(sizeReq, inUse->NextAvailableIndex, m_BlockInUse);
		inUse->NextAvailableIndex += sizeReq;
		return result;
	}

	void MemoryPool::findNewBlockInUse(const uint32_t sizeReq)
	{
		for (size_t i = 0; i < m_Blocks.size(); ++i)
		{
			if (m_Blocks[i].NextAvailableIndex + sizeReq <= m_BlockSize)
			{
				m_BlockInUse = static_cast<uint8_t>(i);
				return;
			}
		}
		// No sufficient block was found, create new
		createBlock();
	}

	MemoryPool::Metadata MemoryPool::readMetadata(const void* data)
	{
		Metadata result;
		const void* metadataPtr = (uint8_t*)data - Metadata::SizeTight();
		memcpy(&result, metadataPtr, Metadata::SizeTight());
		return result;
	}

	void MemoryPool::writeMetadata(void* data, const Metadata& metadata)
	{
		void* metadataPtr = (uint8_t*)data - Metadata::SizeTight();
		memcpy(metadataPtr, &metadata, Metadata::SizeTight());
	}

	constexpr uint32_t MemoryPool::Metadata::SizeTight()
	{
		#ifdef XYZ_DEBUG
		return 3 * static_cast<uint32_t>(sizeof(uint32_t))
			+ static_cast<uint32_t>(sizeof(uint8_t));
		#else
		return 2 * static_cast<uint32_t>(sizeof(uint32_t))
			+ static_cast<uint32_t>(sizeof(uint8_t));
		#endif
	}
}