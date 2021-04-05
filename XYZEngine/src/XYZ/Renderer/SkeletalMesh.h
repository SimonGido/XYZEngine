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

	struct Bone
	{
		glm::mat4 FinalTransform;
		glm::mat4 Transform;
		int32_t ID;
	};

	class SkeletalMesh : public Asset
	{
	public:
		SkeletalMesh(
			const std::vector<AnimatedVertex>& vertices, 
			const std::vector<uint32_t>& indices,
			const std::vector<Bone>& bones,
			const Tree& hierarchy,
			Ref<Material> material
		);
		SkeletalMesh(
			std::vector<AnimatedVertex>&& vertices, 
			std::vector<uint32_t>&& indices,
			std::vector<Bone>&& bones,
			Tree&& hierarchy,
			Ref<Material> material
		);


		void Render(const glm::mat4& viewProjectionMatrix);
		void RebuildBuffers();

		const Ref<Material>& GetMaterial() const { return m_Material; }
		const std::vector<AnimatedVertex>& GetVertices() const { return m_Vertices; }
		const std::vector<uint32_t>& GetIndicies() const { return m_Indices; }
		const std::vector<Bone>& GetBones() const { return m_Bones; }
		const Tree& GetBoneHierarchy() const { return m_BoneHierarchy; }


	private:
		Ref<VertexArray> m_VertexArray;
		Ref<Material> m_Material;

		std::vector<glm::mat4> m_FinalTransformations;

		std::vector<AnimatedVertex> m_Vertices;
		std::vector<uint32_t> m_Indices;	
		std::vector<Bone> m_Bones;
		Tree m_BoneHierarchy;
	};
}