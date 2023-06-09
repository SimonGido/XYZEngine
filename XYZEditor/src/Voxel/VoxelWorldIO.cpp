#include "stdafx.h"
#include "VoxelWorldIO.h"



namespace XYZ {

	static uint32_t Index2D(uint32_t x, uint32_t z, uint32_t depth)
	{
		return x * depth + z;
	}

	template <typename T>
	static T Read(std::fstream& stream)
	{
		T val;
		stream.read((char*)&val, sizeof(T));
		return val;
	}



	VoxelWorldIO::VoxelWorldIO(const std::filesystem::path& path, uint32_t seed)
		:
		m_Path(path),
		m_Seed(seed),
		m_FileStream(path.c_str(), std::ios::binary | std::ios::out | std::ios::in)
	{
	}
	void VoxelWorldIO::Save(const std::vector<VoxelChunkIO>& chunks)
	{
		uint32_t numChunks = Read<uint32_t>(m_FileStream);
	
		for (auto& chunk : chunks)
		{
			
		}
	}
}
