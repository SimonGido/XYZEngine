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
			memset(IDs, 0, sc_MaxBonesPerVertex * sizeof(uint32_t));
			memset(StartWeights, 0, sc_MaxBonesPerVertex * sizeof(float));
			memset(EndWeights, 0, sc_MaxBonesPerVertex * sizeof(float));
		}
		uint32_t IDs[sc_MaxBonesPerVertex];
		float    StartWeights[sc_MaxBonesPerVertex];
		float	 EndWeights[sc_MaxBonesPerVertex];
	};

	struct AnimatedVertex
	{
		glm::vec3      Position;
		glm::vec2	   TexCoord;
		VertexBoneData BoneData;
	};

	struct Joint
	{
		Joint(const glm::vec3& position);

		glm::vec3 Position;
		glm::vec3 FinalPosition;
		glm::vec3 DefaultPosition;
		int32_t ID;
	};

	struct Skeleton
	{
		Tree JointHierarchy;
		std::vector<Joint> Joints;
	};

	struct KeyFrame
	{
		struct Data
		{
			Joint* Joint;
			glm::vec3 StartPosition;
			glm::vec3 EndPosition;
		};

		std::vector<Data> AffectedJoints;
		float Length;
	};

	struct SkeletalAnimation
	{
		SkeletalAnimation() = default;
		SkeletalAnimation(const SkeletalAnimation& other);

		Skeleton Skeleton;
		std::vector<KeyFrame> KeyFrames;
	};

	class SkeletalMesh : public RefCount
	{
	public:
		SkeletalMesh(const SkeletalAnimation& animation);

		void Update(float ts);

	private:
		std::vector<AnimatedVertex> m_Vertices;


		SkeletalAnimation m_Animation;
		uint32_t m_CurrentFrame = 0;

		float m_CurrentTime = 0.0f;
		float m_Repeat = true;
	};
}