#pragma once
#include "XYZ/Renderer/VoxelMesh.h"


#include <glm/glm.hpp>

namespace XYZ {
	struct VoxelChunkIO
	{
		std::string		Biom;
		glm::i64vec2	Coords;
		Ref<VoxelProceduralMesh> Mesh;
	};

	class VoxelWorldIO
	{
	public:
		VoxelWorldIO(const std::filesystem::path& path, uint32_t seed);

		void Save(const std::vector<VoxelChunkIO>& chunks);


	private:
		std::filesystem::path m_Path;
		uint32_t			  m_Seed;
		std::fstream		  m_FileStream;
	};

	
}
