#pragma once
#include "SkinnedMesh.h"
#include "PreviewBone.h"
#include "XYZ/Utils/DataStructures/Tree.h"

namespace XYZ {
	namespace Editor {
		class PreviewRenderer
		{
		public:
			static void RenderSkinnedMesh(const SkinnedMesh& mesh, bool preview);
			static void RenderHierarchy(const Tree& hierarchy, bool transform);
			static void RenderBone(const glm::vec2& start, const glm::vec2& end, const glm::vec2& normal, const glm::vec4& color, float radius );
			static void RenderTriangle(const glm::vec2& firstPosition, const glm::vec2& secondPosition, const glm::vec2& thirdPosition, const glm::vec4& color);
		};
	}
}