#pragma once

#include "XYZ/Core/Ref/Ref.h"


#include "XYZ/Asset/Renderer/VoxelMeshSource.h"
#include "XYZ/Asset/AssetReference.h"

#include <glm/glm.hpp>


namespace XYZ {

	struct VoxelMeshAccelerationGrid
	{
		uint32_t  Width;
		uint32_t  Height;
		uint32_t  Depth;
		float	  Size;

		std::vector<uint32_t> Cells;
	};

	class XYZ_API VoxelMesh : public Asset
	{
	public:
		virtual ~VoxelMesh() = default;
		virtual const std::array<VoxelColor, 256>& GetColorPallete() const = 0;
		virtual const std::vector<VoxelSubmesh>& GetSubmeshes() const = 0;
		virtual const std::vector<VoxelInstance>& GetInstances() const = 0;
		virtual const AssetHandle& GetRenderID() const = 0;
		virtual uint32_t GetNumVoxels() const = 0;

	protected:
		struct DirtyRange
		{
			uint32_t Start = std::numeric_limits<uint32_t>::max();
			uint32_t End = 0;
		};


		virtual bool NeedUpdate() const = 0;
		virtual std::unordered_map<uint32_t, DirtyRange> DirtySubmeshes() const { return {}; }
		
		virtual const VoxelMeshAccelerationGrid& GetAccelerationGrid() const { return {}; }
		virtual bool HasAccelerationGrid() const { return false; }

		friend class VoxelRenderer;
	};

	class XYZ_API VoxelSourceMesh : public VoxelMesh
	{
	public:
		VoxelSourceMesh(const Ref<VoxelMeshSource>& meshSource);
		VoxelSourceMesh(const AssetHandle& meshSourceHandle);
		
		virtual AssetType GetAssetType() const override { return AssetType::VoxelSourceMesh; }


		AssetReference<VoxelMeshSource> GetMeshSource() const { return m_MeshSource; }
		
		virtual const std::array<VoxelColor, 256>& GetColorPallete() const override { return m_MeshSource->GetColorPallete(); }
		virtual const std::vector<VoxelSubmesh>& GetSubmeshes() const override;
		virtual const std::vector<VoxelInstance>& GetInstances() const override;
		virtual const AssetHandle& GetRenderID() const override;
		virtual uint32_t GetNumVoxels() const override { return m_MeshSource->GetNumVoxels(); }
		
		static AssetType GetStaticType() { return AssetType::VoxelSourceMesh; }
	
	private:
		virtual bool NeedUpdate() const override;

	private:
		AssetReference<VoxelMeshSource> m_MeshSource;
	};

	class XYZ_API VoxelProceduralMesh : public VoxelMesh
	{
	public:
		VoxelProceduralMesh();

		virtual AssetType GetAssetType() const override { return AssetType::None; }

		void SetSubmeshes(const std::vector<VoxelSubmesh>& submeshes);
		void SetInstances(const std::vector<VoxelInstance>& instances);
		void SetColorPallete(const std::array<VoxelColor, 256>& pallete);

		bool GenerateAccelerationGrid(float size);
		bool GenerateAccelerationGridAsync(float size);


		void SetVoxelColor(uint32_t submeshIndex, uint32_t x, uint32_t y, uint32_t z, uint8_t value);


		virtual const std::array<VoxelColor, 256>& GetColorPallete() const override;
		virtual const std::vector<VoxelSubmesh>& GetSubmeshes() const override;
		virtual const std::vector<VoxelInstance>& GetInstances() const override;
		virtual const AssetHandle& GetRenderID() const override;
		virtual uint32_t GetNumVoxels() const override { return m_NumVoxels; }
	
		static AssetType GetStaticType() { return AssetType::None; }

	private:
		virtual bool NeedUpdate() const override;
		virtual bool HasAccelerationGrid() const override;
		virtual const VoxelMeshAccelerationGrid& GetAccelerationGrid() const override { return m_AccelerationGrid; }
		virtual std::unordered_map<uint32_t, DirtyRange> DirtySubmeshes() const override;
	
	
		static VoxelMeshAccelerationGrid generateAccelerationGrid(
			const std::vector<VoxelSubmesh>& submeshes,
			const std::array<VoxelColor, 256>& colorPallete,
			float size
		);
	private:
		std::vector<VoxelSubmesh>	m_Submeshes;
		std::vector<VoxelInstance>	m_Instances;
		std::array<VoxelColor, 256> m_ColorPallete;


		VoxelMeshAccelerationGrid	m_AccelerationGrid;
		bool						m_HasAccelerationGrid;
		bool						m_GeneratingAccelerationGrid;
		
		mutable std::unordered_map<uint32_t, DirtyRange> m_DirtySubmeshes;		
		mutable std::atomic_bool m_Dirty;

		uint32_t m_NumVoxels;
	};
}