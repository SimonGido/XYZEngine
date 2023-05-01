#include "stdafx.h"
#include "SpaceColonization.h"

#include "XYZ/Utils/Random.h"

#include <glm/gtc/constants.hpp>

namespace XYZ {
	void SpaceColonization::GenerateAttractors(uint32_t count, uint32_t radius, const glm::vec3& offset)
	{
		for (uint32_t i = 0; i < count; i++)
		{
			float radiusDist = RandomNumber(0.0f, 1.0f);
			
			radiusDist = pow(sin(radiusDist * glm::pi<float>() / 2.0f), 0.8f) * radius;
			// 2 angles are generated from which a direction will be computed
			float alpha = RandomNumber(0.0f, glm::pi<float>());
			float theta = RandomNumber(0.0f, glm::pi<float>() * 2.0f);

			glm::vec3 pt = glm::vec3(
				radiusDist * cos(theta) * sin(alpha),
				radiusDist * sin(theta) * sin(alpha),
				radiusDist * cos(alpha)
			);
			m_Attractors.push_back(offset + pt);
		}
	}
	void SpaceColonization::Update()
	{
		uint32_t maxUpdates = 50;
		uint32_t updateCount = 0;
		while (!m_Attractors.empty() && updateCount < maxUpdates)
		{
			if (m_AllBranches.empty())
			{
				auto& branch = m_ActiveBranches.emplace_back();
				branch.Start = glm::ivec3(0);
			}

			std::vector<Branch> newActiveBranches;
			for (auto& branch : m_ActiveBranches)
			{
				uint32_t attractionIndex = findAttractorIndex(branch.Start);
				if (attractionIndex != sc_InvalidIndex)
				{
					branch.End = m_Attractors[attractionIndex];
					m_Attractors.erase(m_Attractors.begin() + attractionIndex);
				}
				else
				{
					branch.End = randomGrowthVector();
				}
				m_AllBranches.push_back(branch);
				auto& newBranch = newActiveBranches.emplace_back();
				newBranch.Start = branch.End;
			}
			m_ActiveBranches = std::move(newActiveBranches);

			updateCount++;
		}

	}
	glm::vec3 SpaceColonization::randomGrowthVector() const
	{
		float alpha = RandomNumber(0.0f, glm::pi<float>());
		float theta = RandomNumber(0.0f, glm::pi<float>() * 2.0f);

		return glm::vec3(
			cos(theta) * sin(alpha),
			sin(theta) * sin(alpha),
			cos(alpha)
		) * RandomGrowth;
	}
	uint32_t SpaceColonization::findAttractorIndex(const glm::ivec3& pos) const
	{
		float minDistance = std::numeric_limits<float>::max();
		uint32_t result = sc_InvalidIndex;
		for (uint32_t i = 0; i < m_Attractors.size(); ++i)
		{
			minDistance = std::min(minDistance, glm::distance(glm::vec3(m_Attractors[i]), glm::vec3(pos)));
			if (minDistance < AttractionRange)
			{
				result = i;
			}
		}
		return result;
	}
}