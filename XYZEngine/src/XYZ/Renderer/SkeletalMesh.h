#pragma once
#include "XYZ/Core/Ref.h"
#include "XYZ/Core/MemoryPool.h"
#include "XYZ/Renderer/VertexArray.h"
#include "XYZ/Renderer/Material.h"
#include "XYZ/Utils/DataStructures/Tree.h"


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
			memset(IDs, -1, sc_MaxBonesPerVertex * sizeof(int32_t));
		}

		int32_t IDs[sc_MaxBonesPerVertex];
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
		float Length = 3.0f;
		int32_t ID;

		// Debug 
		std::string Name;
	};

	struct Skeleton
	{
		Skeleton() = default;
		Skeleton(const Skeleton& other);

		Tree BoneHierarchy;
		std::vector<Bone> Bones;
	};

	class SkeletalMesh : public RefCount
	{
	public:
		SkeletalMesh(const Skeleton& skeleton);

		void Update(float ts);

		void Render();
	//private:
		std::vector<AnimatedVertex> m_Vertices;

		Bone* m_Selected = nullptr;
		glm::vec2 m_OldMousePosition = glm::vec2(0.0f);

		Skeleton m_Skeleton;
		uint32_t m_CurrentFrame = 0;

		float m_CurrentTime = 0.0f;
		float m_Repeat = true;
	};
}