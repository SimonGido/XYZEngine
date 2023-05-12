#pragma once

#include <glm/glm.hpp>

namespace XYZ {
	struct SCBranch
	{
		glm::vec3	Start;
		glm::vec3   End;
		glm::vec3	Direction;
		SCBranch*   Parent;				// Pointer to parent of the branch
		uint32_t	DistanceFromRoot = 0;
		bool		Grown = false;

		std::vector<SCBranch*> Children;
		std::vector<glm::vec3> Attractors;
	};

	struct SCInitializer
	{
		uint32_t	AttractorsCount;
		glm::vec3	AttractorsCenter;
		float		AttractorsRadius;
		float		AttractionRange;
		float		KillRange;

		glm::vec3	RootPosition;
		float		BranchLength;
	};

	class XYZ_API SpaceColonization
	{
	public:
		SpaceColonization(const SCInitializer& initializer);

		void Grow();
		void Grow(std::vector<uint8_t>& voxels, uint32_t width, uint32_t height, uint32_t depth, float voxelSize, int32_t radius);
		void VoxelizeAttractors(std::vector<uint8_t>& voxels, uint32_t width, uint32_t height, uint32_t depth, float voxelSize);

	private:
		static glm::vec3 randomGrowVector(float growSize);


	private:
		std::vector<glm::vec3> m_Attractors;
		std::vector<uint32_t>  m_ActiveAttractors;
		std::vector<SCBranch*> m_Extremities;
		std::vector<SCBranch*> m_Branches;

		SCBranch* m_RootBranch;
		float     m_BranchLength;
		float	  m_KillRange;
		float	  m_AttractionRange;
		float	  m_GrowSize = 0.1f;
	};
}
