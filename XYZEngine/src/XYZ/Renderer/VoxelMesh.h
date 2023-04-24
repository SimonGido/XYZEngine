#pragma once

#include "XYZ/Core/Ref/Ref.h"


#include "XYZ/Asset/Renderer/VoxelMeshSource.h"
#include "XYZ/Asset/AssetReference.h"

#include <glm/glm.hpp>


namespace XYZ {


	class XYZ_API VoxelMesh : public Asset
	{
	public:
		virtual ~VoxelMesh() = default;
		virtual const std::vector<VoxelSubmesh>& GetSubmeshes() const = 0;
		virtual const std::vector<VoxelInstance>& GetInstances() const = 0;
		virtual const AssetHandle& GetRenderID() const = 0;
		virtual uint32_t GetNumVoxels() const = 0;

	protected:
		virtual bool NeedUpdate() const = 0;

		friend class VoxelRenderer;
	};

	class XYZ_API VoxelSourceMesh : public VoxelMesh
	{
	public:
		VoxelSourceMesh(const Ref<VoxelMeshSource>& meshSource);
		VoxelSourceMesh(const AssetHandle& meshSourceHandle);
		
		virtual AssetType GetAssetType() const override { return AssetType::VoxelSourceMesh; }


		AssetReference<VoxelMeshSource> GetMeshSource() const { return m_MeshSource; }

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

		virtual const std::vector<VoxelSubmesh>& GetSubmeshes() const override;
		virtual const std::vector<VoxelInstance>& GetInstances() const override;
		virtual const AssetHandle& GetRenderID() const override;
		virtual uint32_t GetNumVoxels() const override { return m_NumVoxels; }
		
		static AssetType GetStaticType() { return AssetType::None; }

	private:
		virtual bool NeedUpdate() const override;

	private:
		std::vector<VoxelSubmesh> m_Submeshes;
		std::vector<VoxelInstance> m_Instances;

		uint32_t m_NumVoxels;
		mutable bool m_Dirty;
	};
}