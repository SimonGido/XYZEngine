#include "stdafx.h"
#include "SpaceColonization.h"

#include "XYZ/Utils/Random.h"
#include "XYZ/Utils/Algorithms/Raymarch.h"

#include <glm/ext/scalar_constants.hpp>

namespace XYZ {

	static uint32_t Index3D(const glm::ivec3& index, uint32_t width, uint32_t height)
	{
		return index.x + width * (index.y + height * index.z);
	}

	SpaceColonization::SpaceColonization(
		uint32_t numAttractors,
		const glm::vec3& attractorsCenter, 
		float attractorsRadius,
		const glm::vec3& rootPosition,
		float branchLength,
		float killRange,
		float attractionRange
	)
		:
		m_BranchLength(branchLength),
		m_KillRange(killRange),
		m_AttractionRange(attractionRange)
	{
		m_Attractors.resize(numAttractors);
		for (uint32_t i = 0; i < numAttractors; i++)
		{
			float theta = 2.0 * glm::pi<float>() * RandomNumber(0.0f, 1.0f);      // Random angle around the vertical axis
			float phi = acos(2.0 * RandomNumber(0.0f, 1.0f) - 1.0);   // Random angle from the vertical axis

			// Convert spherical coordinates to Cartesian coordinates
			m_Attractors[i].x = attractorsCenter.x + RandomNumber(0.0f, attractorsRadius) * sin(phi) * cos(theta);
			m_Attractors[i].y = attractorsCenter.y + RandomNumber(0.0f, attractorsRadius) * sin(phi) * sin(theta);
			m_Attractors[i].z = attractorsCenter.z + RandomNumber(0.0f, attractorsRadius) * cos(phi);
		}
		m_RootBranch = new SCBranch();
		m_RootBranch->Start = rootPosition;
		m_RootBranch->End = rootPosition + glm::vec3(0.0f, branchLength, 0.0f);
		m_RootBranch->Direction = glm::vec3(0.0f, 1.0f, 0.0f);
		m_RootBranch->Parent = nullptr;
		m_Branches.push_back(m_RootBranch);
		m_Extremities.push_back(m_RootBranch);
	}
	void SpaceColonization::Grow()
	{
		// we parse the extremities to set them as grown 
		for (SCBranch* extremity : m_Extremities) 
			extremity->Grown = true;
		
		// we remove the attractors in kill range
		for (int32_t i = m_Attractors.size() - 1; i >= 0; i--) 
		{
			for (SCBranch* branch : m_Branches)
			{
				if (glm::distance(branch->End, m_Attractors[i]) < m_KillRange) 
				{
					m_Attractors.erase(m_Attractors.begin() + i);
					break;
				}
			}
		}

		if (!m_Attractors.empty())
		{
			// we clear the active attractors
			m_ActiveAttractors.clear();
			for (SCBranch* branch : m_Branches)
			{
				branch->Attractors.clear();
			}

			// each attractor is associated to its closest branch, if in attraction range
			uint32_t attractorIndex = 0;
			for (const glm::vec3& attractor : m_Attractors) 
			{
				float min = std::numeric_limits<float>::max();
				SCBranch* closest = nullptr; // will store the closest branch
				for (SCBranch* branch : m_Branches) 
				{
					float distance = glm::distance(branch->End, attractor);
					if (distance < m_AttractionRange && distance < min)
					{
						min = distance;
						closest = branch;
					}
				}

				// if a branch has been found, we add the attractor to the branch
				if (closest != nullptr) 
				{
					closest->Attractors.push_back(attractor);
					m_ActiveAttractors.push_back(attractorIndex);
				}
				attractorIndex++;
			}


			// if at least an attraction point has been found, we want our tree to grow towards it
			if (!m_ActiveAttractors.empty()) 
			{
				// because new extremities will be set here, we clear the current ones
				m_Extremities.clear();

				// new branches will be added here
				std::vector<SCBranch*> newBranches;
				for (SCBranch* branch : m_Branches) 
				{
					// if the branch has attraction points, we grow towards them
					if (!branch->Attractors.empty())
					{
						// we compute the direction of the new branch
						glm::vec3 dir(0, 0, 0);
						for (const glm::vec3& attr : branch->Attractors) 
						{
							dir += glm::normalize(attr - branch->End);
						}
						dir /= branch->Attractors.size();
						// random growth
						dir += randomGrowVector(m_GrowSize);
						dir = glm::normalize(dir);

						// our new branch grows in the correct direction
						SCBranch* newBranch = new SCBranch();
						newBranch->Start = branch->End;
						newBranch->End = branch->End + dir * m_BranchLength;
						newBranch->Direction = dir;
						newBranch->Parent = branch;


						newBranch->DistanceFromRoot = branch->DistanceFromRoot + 1;
						branch->Children.push_back(newBranch);
						newBranches.push_back(newBranch);
						m_Extremities.push_back(newBranch);
					}
					else 
					{
						// if no attraction points, we only check if the branch is an extremity
						if (branch->Children.empty()) 
						{
							m_Extremities.push_back(branch);
						}
					}
				}

				// we merge the new branches with the previous ones
				for (auto newBranch : newBranches)
					m_Branches.push_back(newBranch);
			}
			else 
			{
				// we grow the extremities of the tree
				for (uint32_t i = 0; i < m_Extremities.size(); ++i) 
				{
					SCBranch* extremity = m_Extremities[i];
					// the new branch starts where the extremity ends
					glm::vec3 start = extremity->End;
					// we add randomness to the direction
					glm::vec3 dir = extremity->Direction + randomGrowVector(m_GrowSize);
					// we add the direction multiplied by the branch length to get the end point
					glm::vec3 end = extremity->End + dir * m_BranchLength;
					// a new branch can be created with the same direction as its parent
					SCBranch* newBranch = new SCBranch();
					newBranch->Start = start;
					newBranch->End = end;
					newBranch->Direction = dir;
					newBranch->Parent = extremity;

					// the current extrimity has a new child
					extremity->Children.push_back(newBranch);

					// let's add the new branch to the list and set it as the new extremity 
					m_Branches.push_back(newBranch);
					
					m_Extremities[i] = newBranch;
				}
			}
		}
	}
	void SpaceColonization::Grow(std::vector<uint8_t>& voxels, uint32_t width, uint32_t height, uint32_t depth, float voxelSize)
	{
		Grow();

		const uint32_t Wood = 4;
		for (const SCBranch * branch : m_Branches)
		{
			Ray ray(branch->Start, branch->Direction);
			Raymarch raymarch(ray, width, height, depth, voxelSize);
			
			const glm::vec3 startVoxelPosition = glm::vec3(raymarch.GetStartVoxel()) * voxelSize;
			glm::vec3 currentVoxelPosition = glm::vec3(raymarch.GetCurrentVoxel()) * voxelSize;
			while (glm::distance(currentVoxelPosition, startVoxelPosition) < m_BranchLength)
			{
				const uint32_t index = Index3D(raymarch.GetCurrentVoxel(), width, height);
				if (index < voxels.size())
					voxels[index] = Wood;

				raymarch.Step();
				currentVoxelPosition = glm::vec3(raymarch.GetCurrentVoxel()) * voxelSize;
			}
		}
	}

	void SpaceColonization::VoxelizeAttractors(std::vector<uint8_t>& voxels, uint32_t width, uint32_t height, uint32_t depth, float voxelSize)
	{
		for (const auto& attractor : m_Attractors)
		{
			glm::ivec3 voxel(attractor / voxelSize);
			const uint32_t index = Index3D(voxel, width, height);
			if (index < voxels.size())
				voxels[index] = 10;
		}
	}
	glm::vec3 SpaceColonization::randomGrowVector(float growSize)
	{
		float alpha = RandomNumber(0.0f, glm::pi<float>());
		float theta = RandomNumber(0.0f, glm::pi<float>() * 2.0f);

		glm::vec3 point(
			cos(theta) * sin(alpha),
			sin(theta) * sin(alpha),
			cos(alpha)
		);

		return point * growSize;
	}

}
