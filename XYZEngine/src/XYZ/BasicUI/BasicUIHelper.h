#pragma once
#include "XYZ/Utils/DataStructures/Tree.h"
#include "XYZ/Renderer/Font.h"

#include <glm/glm.hpp>

namespace XYZ {

	struct bUILayout;
	class bUIHelper
	{
	public:
		static glm::vec2 FindTextSize(const char* source, const Ref<Font>& font);
		static void ResolvePosition(int32_t elementID, Tree& tree, const bUILayout& layout);
	};
}