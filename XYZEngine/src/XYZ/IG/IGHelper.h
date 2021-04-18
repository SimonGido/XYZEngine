#pragma once
#include "IGElement.h"

namespace XYZ {

	class IGHelper
	{
	public:
		static void PopFromMesh(IGMesh& mesh, size_t start, size_t end);
		static void ResolvePosition(
			const glm::vec2& border,
			const glm::vec2& genSize,
			const IGStyle& style,
			glm::vec2& offset,
			IGElement& element,
			IGMesh& mesh,
			float& maxY,
			size_t oldQuadCount,
			size_t oldLineCount
		);

		static glm::vec2 GetBorder(const IGElement& element);
		
		enum InsideFlags
		{
			Left   = BIT(0),
			Right  = BIT(1),
			Top    = BIT(2),
			Bottom = BIT(3)
		};

		static bool IsInside(const glm::vec2& parentPos, const glm::vec2& parentSize, const glm::vec2& pos, const glm::vec2& size, uint8_t flags);
		
	};

}