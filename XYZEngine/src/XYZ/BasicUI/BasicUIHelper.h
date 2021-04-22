#pragma once
#include "XYZ/Utils/DataStructures/Tree.h"
#include "XYZ/Renderer/Font.h"

#include <glm/glm.hpp>

namespace XYZ {

	struct bUILayout;
	class bUIHelper
	{
	public:
		static glm::vec2 FindTextSize(const char* source, const Ref<Font>& font, uint32_t maxCharacters = UINT32_MAX);
		static uint32_t FindNumCharacterToFit(const glm::vec2& size, const char* source, const Ref<Font>& font);
		static void ResolvePosition(int32_t elementID, Tree& tree, const bUILayout& layout);
		static bool IsInside(const glm::vec2& posA, const glm::vec2& sizeA, const glm::vec2& posB, const glm::vec2& sizeB);
	};
}