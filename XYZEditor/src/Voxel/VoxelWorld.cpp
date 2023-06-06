#include "stdafx.h"
#include "VoxelWorld.h"

#include "XYZ/Utils/Math/Perlin.h"

namespace XYZ {

	static uint32_t Index3D(uint32_t x, uint32_t y, uint32_t z, uint32_t width, uint32_t height)
	{
		return x + width * (y + height * z);
	}

	static uint32_t Index3D(const glm::uvec3& index, uint32_t width, uint32_t height)
	{
		return index.x + width * (index.y + height * index.z);
	}

	static uint32_t Index2D(uint32_t x, uint32_t z, uint32_t depth)
	{
		return x * depth + z;
	}

	VoxelWorld::VoxelWorld(const std::filesystem::path& worldPath, uint32_t seed)
		:
		m_WorldPath(worldPath),
		m_Seed(seed)
	{
		VoxelBiom& forestBiom = m_Bioms["Forest"];
		forestBiom.ColorPallete[0] = { 0, 0, 0, 0 };
		forestBiom.ColorPallete[1] = { 1, 60, 32, 255 }; // Grass
		forestBiom.Octaves = 1;
		forestBiom.Frequency = 2.0f;

		Perlin::SetSeed(seed);
		generateChunks(0, 0);
	}
	void VoxelWorld::Update(const glm::vec3& position)
	{
		constexpr uint32_t halfDimensionX = sc_ChunkDimensions.x / 2;
		constexpr uint32_t halfDimensionZ = sc_ChunkDimensions.z / 2;


		for (auto it = m_ChunksGenerated.begin(); it != m_ChunksGenerated.end(); )
		{
			std::shared_ptr<GeneratedChunk> chunk = (*it);
			if (chunk->Finished)
			{
				m_ActiveChunks[chunk->IndexX][chunk->IndexZ] = std::move(chunk->Chunk);
				it = m_ChunksGenerated.erase(it);
			}
			else
			{
				it++;
			}
		}


		const int64_t centerChunkX = static_cast<int64_t>(std::floor((position.x + halfDimensionX) / sc_ChunkDimensions.x));
		const int64_t centerChunkZ = static_cast<int64_t>(std::floor((position.z + halfDimensionZ) / sc_ChunkDimensions.z));
		
		const int64_t shiftDirX = m_LastCenterChunkX - centerChunkX;
		const int64_t shiftDirZ = m_LastCenterChunkZ - centerChunkZ;
		if (shiftDirX == 0 && shiftDirZ == 0)
			return;

		
		if (shiftDirX != 0 || shiftDirZ != 0)
			m_ActiveChunks = std::move(shiftChunks(shiftDirX, shiftDirZ));

		m_ChunksGenerated.clear();
		generateChunks(centerChunkX, centerChunkZ);
		m_LastCenterChunkX = centerChunkX;
		m_LastCenterChunkZ = centerChunkZ;
	}
	void VoxelWorld::generateChunks(int64_t centerChunkX, int64_t centerChunkZ)
	{
		const int64_t chunksWidth = sc_MaxVisibleChunksPerAxis;

		const int64_t chunkMinCoordX = centerChunkX - sc_ChunkViewDistance;
		const int64_t chunkMaxCoordX = centerChunkX + sc_ChunkViewDistance;

		const int64_t chunkMinCoordZ = centerChunkZ - sc_ChunkViewDistance;
		const int64_t chunkMaxCoordZ = centerChunkZ + sc_ChunkViewDistance;
	
		auto& pool = Application::Get().GetThreadPool();
		VoxelBiom& forestBiom = m_Bioms["Forest"];
		for (int64_t chunkX = 0; chunkX < chunksWidth; chunkX++)
		{
			for (int64_t chunkZ = 0; chunkZ < chunksWidth; chunkZ++)
			{
				const int64_t worldChunkX = chunkX + centerChunkX - sc_ChunkViewDistance;
				const int64_t worldChunkZ = chunkZ + centerChunkZ - sc_ChunkViewDistance;

				if (!m_ActiveChunks[chunkX][chunkZ].Mesh.Raw()) // Chunk was shifted away
				{
					m_ChunksGenerated.push_back(std::make_shared<GeneratedChunk>());
					std::shared_ptr<GeneratedChunk> gen = m_ChunksGenerated.back();
					gen->IndexX = chunkX;
					gen->IndexZ = chunkZ;
					pool.PushJob([gen, worldChunkX, worldChunkZ, &forestBiom]() {
						gen->Chunk = generateChunk(worldChunkX, worldChunkZ, forestBiom);
						gen->Finished = true;
					});
				}
			}
		}
	}
	VoxelWorld::ActiveChunkStorage VoxelWorld::shiftChunks(int64_t dirX, int64_t dirZ)
	{
		ActiveChunkStorage shiftedChunks;
		if (dirX > sc_ChunkViewDistance || dirZ > sc_ChunkViewDistance)
			return shiftedChunks;

		const int64_t chunksWidth = sc_MaxVisibleChunksPerAxis;
		for (int64_t chunkX = 0; chunkX < chunksWidth; chunkX++)
		{
			const int64_t shiftedChunkX = chunkX + dirX;
			if (shiftedChunkX >= sc_MaxVisibleChunksPerAxis || shiftedChunkX < 0)
				continue;

			for (int64_t chunkZ = 0; chunkZ < chunksWidth; chunkZ++)
			{
				const int64_t shiftedChunkZ = chunkZ + dirZ;
				if (shiftedChunkZ >= sc_MaxVisibleChunksPerAxis || shiftedChunkZ < 0)
					continue;

				shiftedChunks[shiftedChunkX][shiftedChunkZ] = std::move(m_ActiveChunks[chunkX][chunkZ]);
			}
		}
		return shiftedChunks;
	}
	VoxelChunk VoxelWorld::generateChunk(int64_t chunkX, int64_t chunkZ, const VoxelBiom& biom)
	{
		VoxelChunk chunk;		
		chunk.X = chunkX;
		chunk.Z = chunkZ;

		chunk.Mesh = Ref<VoxelProceduralMesh>::Create();
		chunk.Mesh->SetColorPallete(biom.ColorPallete);

		VoxelSubmesh submesh;
		submesh.Width = sc_ChunkDimensions.x;
		submesh.Height = sc_ChunkDimensions.y;
		submesh.Depth = sc_ChunkDimensions.z;
		submesh.VoxelSize = sc_ChunkVoxelSize;
		
		const glm::vec3 centerTranslation = -glm::vec3(
			submesh.Width / 2.0f * submesh.VoxelSize,
			submesh.Height / 2.0f * submesh.VoxelSize,
			submesh.Depth / 2.0f * submesh.VoxelSize
		);

		const glm::vec3 translation = glm::vec3(
			chunk.X * sc_ChunkDimensions.x * sc_ChunkVoxelSize, 
			0.0f, 
			chunk.Z * sc_ChunkDimensions.z * sc_ChunkVoxelSize
		);

		VoxelInstance instance;
		instance.SubmeshIndex = 0;
		instance.Transform = glm::translate(glm::mat4(1.0f), translation + centerTranslation);

		const double fx = static_cast<double>(biom.Frequency / submesh.Width);
		const double fz = static_cast<double>(biom.Frequency / submesh.Depth);

		submesh.ColorIndices.resize(submesh.Width * submesh.Height * submesh.Depth, 0);
		submesh.RendererCopy = new uint8_t[submesh.ColorIndices.size()];
		for (uint32_t x = 0; x < submesh.Width; ++x)
		{
			for (uint32_t z = 0; z < submesh.Depth; ++z)
			{
				const double xDouble = static_cast<double>(x) + chunk.X * sc_ChunkDimensions.x;
				const double zDouble = static_cast<double>(z) + chunk.Z * sc_ChunkDimensions.z;
				const double val = Perlin::Octave2D(xDouble * fx, zDouble * fz, biom.Octaves);
				const uint32_t genHeight = val * submesh.Height;

				for (uint32_t y = 0; y < genHeight; y++)
				{
					const uint32_t index = Index3D(x, y, z, submesh.Width, submesh.Height);
					submesh.ColorIndices[index] = 1;
					submesh.RendererCopy[index] = submesh.ColorIndices[index];
				}
			}
		}
		submesh.Compress(16);
		chunk.Mesh->SetSubmeshes({ submesh });
		chunk.Mesh->SetInstances({ instance });
		chunk.Mesh->GenerateAccelerationGrid(16);
		return chunk;
	}
}
