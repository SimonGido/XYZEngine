#pragma once
#include "XYZ/Utils/Math/Ray.h"
#include "XYZ/Utils/DataStructures/Octree.h"
#include "XYZ/Renderer/VoxelMesh.h"


#include <glm/glm.hpp>

namespace XYZ{

	struct VoxelChunk
	{
		int64_t X = 0;
		int64_t Z = 0;

		Ref<VoxelProceduralMesh> Mesh;
	};

	struct VoxelBiom
	{
		float	 Frequency;
		uint32_t Octaves;
		std::array<VoxelColor, 256> ColorPallete;
	};

	class VoxelWorld
	{
	public:
		static constexpr glm::ivec3 sc_ChunkDimensions = glm::ivec3(256, 512, 256);
		static constexpr int64_t    sc_MaxVisibleChunksPerAxis = 3;
		static constexpr int64_t    sc_MinVisibleChunks = 1;
		static constexpr float      sc_ChunkVoxelSize = 1.0f;

		using ActiveChunkStorage = std::array<std::array<VoxelChunk, sc_MaxVisibleChunksPerAxis>, sc_MaxVisibleChunksPerAxis>;
	public:
		VoxelWorld(const std::filesystem::path& worldPath, uint32_t seed);
		
		void Update(const glm::vec3& position);

		const ActiveChunkStorage& GetActiveChunks() const { return m_ActiveChunks; }
	private:
		void generateChunks(int64_t centerChunkX, int64_t centerChunkZ);

		ActiveChunkStorage shiftChunks(int64_t dirX, int64_t dirZ);

		VoxelChunk generateChunk(int64_t chunkX, int64_t chunkZ, const VoxelBiom& biom) const;

	private:
		ActiveChunkStorage m_ActiveChunks;

		std::filesystem::path m_WorldPath;

		std::unordered_map<std::string, VoxelBiom> m_Bioms;

		uint32_t m_Seed;

		uint32_t m_ChunkViewDistance = 1; // View distance from center

		int64_t m_LastCenterChunkX = 0;
		int64_t m_LastCenterChunkZ = 0;
	};

}
