#pragma once
#include <glm/glm.hpp>

namespace XYZ {

	struct Branch
	{
		glm::ivec3 Start;
		glm::ivec3 End;
	};

	class SpaceColonization
	{
	public:
		void GenerateAttractors(uint32_t count, uint32_t radius, const glm::vec3& offset);

		void Update();

		float RandomGrowth = 5.0f;
		float AttractionRange = 5.0f;
	private:
		glm::vec3 randomGrowthVector() const;
		uint32_t findAttractorIndex(const glm::ivec3& pos) const;

	private:
		std::vector<glm::ivec3> m_Attractors;
		std::vector<Branch> m_AllBranches;
		std::vector<Branch> m_ActiveBranches;

		static constexpr uint32_t sc_InvalidIndex = std::numeric_limits<uint32_t>::max();
	};

}