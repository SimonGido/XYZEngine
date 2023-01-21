#pragma once
#include "XYZ/Core/Ref/Ref.h"

#include "XYZ/Renderer/Material.h"
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Buffer.h"

#include "XYZ/Asset/Renderer/MeshSource.h"

#include <glm/glm.hpp>

namespace XYZ {

	
	class XYZ_API Mesh : public Asset
	{
	public:
		Mesh(const Ref<MeshSource>& meshSource);
		
		virtual ~Mesh() override;
		virtual AssetType GetAssetType() const override { return AssetType::Mesh; }

		Ref<MeshSource>		GetMeshSource() const { return m_MeshSource; }
		Ref<VertexBuffer>   GetVertexBuffer() const { return m_MeshSource->GetVertexBuffer(); }
		Ref<IndexBuffer>    GetIndexBuffer()  const { return m_MeshSource->GetIndexBuffer(); }

		static AssetType	GetStaticType() { return AssetType::Mesh; }

	private:
		Ref<MeshSource>   m_MeshSource;
		// TODO: materials
	};
	
	class XYZ_API AnimatedMesh : public Asset
	{
	public:
		AnimatedMesh(const Ref<MeshSource>& meshSource);
		
		virtual AssetType GetAssetType() const override { return AssetType::AnimatedMesh; }

		Ref<MeshSource>		GetMeshSource() const { return m_MeshSource; }
		Ref<VertexBuffer>   GetVertexBuffer() const { return m_MeshSource->GetVertexBuffer(); }
		Ref<IndexBuffer>    GetIndexBuffer()  const { return m_MeshSource->GetIndexBuffer(); }

		static AssetType	GetStaticType() { return AssetType::AnimatedMesh; }

	private:
		Ref<MeshSource>   m_MeshSource;
		// TODO: materials
	};
}
