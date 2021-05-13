#pragma once
#include "XYZ/Asset/Asset.h"
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Utils/DataStructures/Tree.h"
#include "XYZ/Utils/DataStructures/MemoryPool.h"


#include <glm/glm.hpp>

namespace XYZ {

	struct VertexBoneData
	{
		static constexpr uint32_t sc_MaxBonesPerVertex = 4;
		VertexBoneData()
		{
			Reset();
		}
		VertexBoneData(const VertexBoneData& other)
		{
			memcpy(IDs, other.IDs, sc_MaxBonesPerVertex * sizeof(uint32_t));
			memcpy(Weights, other.Weights, sc_MaxBonesPerVertex * sizeof(float));
		}
		void Reset()
		{
			memset(IDs, 0, sc_MaxBonesPerVertex * sizeof(uint32_t));
			memset(Weights, 0, sc_MaxBonesPerVertex * sizeof(float));
		}

		uint32_t IDs[sc_MaxBonesPerVertex];
		float    Weights[sc_MaxBonesPerVertex];
	};

	struct AnimatedVertex
	{
		glm::vec3      Position;
		glm::vec2	   TexCoord;
		VertexBoneData BoneData;
	};

	//struct Bone
	//{
	//	glm::mat4 Transform;
	//	glm::mat4 WorldTransform;
	//	std::string Name;
	//	int32_t ID;
	//};

	class SceneEntity;
	class SkeletalMesh : public Asset
	{
	public:
		SkeletalMesh(
			const std::vector<AnimatedVertex>& vertices, 
			const std::vector<uint32_t>& indices,
			const Ref<Material>& material
		);
		SkeletalMesh(
			std::vector<AnimatedVertex>&& vertices, 
			std::vector<uint32_t>&& indices,
			Ref<Material>&& material
		);


		void Render();
		void RebuildBuffers();

		const Ref<Material>& GetMaterial() const { return m_Material; }
		const std::vector<AnimatedVertex>& GetVertices() const { return m_Vertices; }
		const std::vector<uint32_t>& GetIndicies() const { return m_Indices; }

	private:
		Ref<VertexArray> m_VertexArray;
		Ref<Material> m_Material;

		std::vector<AnimatedVertex> m_Vertices;
		std::vector<uint32_t> m_Indices;	
		std::vector<SceneEntity> m_Bones;
	};
}