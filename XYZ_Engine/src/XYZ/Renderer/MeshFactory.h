#pragma once
#include "Mesh.h"


#include "XYZ/Gui/Text.h"

namespace XYZ {


	class MeshFactory
	{
	public:
		static Ref<Mesh> CreateTextMesh(const TextUI& text,const glm::vec4& color);
		static Ref<Mesh> CreateSprite(const glm::vec4& color, const glm::vec4& texCoord, int32_t textureID);
	};
}