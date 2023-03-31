#pragma once
#include "XYZ/Core/Ref/Ref.h"

#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/Buffer.h"

#include "XYZ/Asset/Renderer/MeshSource.h"
#include "XYZ/Asset/AssetReference.h"

#include <glm/glm.hpp>

namespace XYZ {
	// NOTE: instead of AssetHandle use some unique id size_t
	using RenderID = AssetHandle;

	class XYZ_API Mesh : public Asset
	{
	public:
		virtual ~Mesh() = default;
		virtual Ref<VertexBuffer>   GetVertexBuffer() const = 0;
		virtual Ref<IndexBuffer>    GetIndexBuffer()  const = 0;		
		virtual const RenderID&		GetRenderID() const = 0;
	};

	
	class XYZ_API StaticMesh : public Mesh
	{
	public:
		StaticMesh(const Ref<MeshSource>& meshSource);
		StaticMesh(const AssetHandle& meshSourceHandle);

		virtual AssetType GetAssetType() const override { return AssetType::StaticMesh; }

		Ref<MeshSource>		GetMeshSource() const { return m_MeshSource.As(); }

		virtual Ref<VertexBuffer>   GetVertexBuffer() const override { return m_MeshSource->GetVertexBuffer(); }
		virtual Ref<IndexBuffer>    GetIndexBuffer()  const override { return m_MeshSource->GetIndexBuffer(); }
		virtual const RenderID&		GetRenderID() const override { return GetHandle(); }


		static AssetType	GetStaticType() { return AssetType::StaticMesh; }

	private:
		AssetReference<MeshSource>   m_MeshSource;
		// TODO: materials
	};


	
	
	class XYZ_API AnimatedMesh : public Mesh
	{
	public:
		AnimatedMesh(const Ref<MeshSource>& meshSource);
		AnimatedMesh(const AssetHandle& meshSourceHandle);

		virtual AssetType GetAssetType() const override { return AssetType::AnimatedMesh; }

		Ref<MeshSource>				GetMeshSource() const { return m_MeshSource.As(); }
		
		virtual Ref<VertexBuffer>   GetVertexBuffer() const override { return m_MeshSource->GetVertexBuffer(); }
		virtual Ref<IndexBuffer>    GetIndexBuffer()  const override { return m_MeshSource->GetIndexBuffer(); }
		virtual const RenderID&		GetRenderID() const override { return GetHandle(); }

		static AssetType	GetStaticType() { return AssetType::AnimatedMesh; }

	private:
		AssetReference<MeshSource>   m_MeshSource;
		// TODO: materials
	};

	class XYZ_API ProceduralMesh : public Mesh
	{
	public:
		ProceduralMesh();

		void CopyVertices(const void* vertices, uint32_t size);
		void CopyIndices(const void* indices, uint32_t count, IndexType type = IndexType::Uint32);

		void MoveVertices(void** vertices, uint32_t size);
		void MoveIndices(void** indices, uint32_t count, IndexType type = IndexType::Uint32);

		virtual Ref<VertexBuffer>   GetVertexBuffer() const override { return m_VertexBuffer; }
		virtual Ref<IndexBuffer>    GetIndexBuffer()  const override { return m_IndexBuffer; }
		virtual const RenderID&		GetRenderID()	  const override { return m_RenderID; }

		virtual AssetType GetAssetType() const override { return AssetType::None; }

	private:
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer>  m_IndexBuffer;
		RenderID		  m_RenderID;
	};
}
