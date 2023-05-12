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


	class SpaceColonization
	{
	public:
		SpaceColonization(
			uint32_t numAttractors,
			const glm::vec3& attractorsCenter,
			float attractorsRadius,
			const glm::vec3& rootPosition,
			float branchLength,
			float killRange,
			float attractionRange
		);


		void Grow();
		void Grow(std::vector<uint8_t>& voxels, uint32_t width, uint32_t height, uint32_t depth, float voxelSize);
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
