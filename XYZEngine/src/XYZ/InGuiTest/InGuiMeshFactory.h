#pragma once
#include "InGuiRenderer.h"


namespace XYZ {

	class IGElement;
	struct IGMeshFactoryData
	{
		uint32_t	  SubTextureIndex;
		IGElement*    Element;
		IGMesh*		  Mesh;
		IGRenderData* RenderData;

		uint32_t	  ScissorIndex = 0;
	};

	class IGMeshFactory
	{
	public:
		template <typename T, typename ...Args>
		static glm::vec2 GenerateUI(const char* label, const glm::vec4& labelColor, const IGMeshFactoryData& data);
	};

}
